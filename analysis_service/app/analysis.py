from __future__ import annotations

import csv
import io
import math
import os
import re
import json
import urllib.error
import urllib.request
import warnings
from datetime import datetime, timezone
from hashlib import sha256
from pathlib import Path
from typing import Any

import pandas as pd

SUPPORTED_EXTENSIONS = {".xlsx", ".xls", ".csv", ".txt"}
TEXT_ENCODINGS = ("utf-8", "utf-8-sig", "gbk", "gb18030", "latin1")


def default_data_dir() -> Path:
    configured = os.getenv("DATA_DIR")
    if configured:
        return Path(configured).resolve()
    return Path(__file__).resolve().parents[2] / "demo"


def list_datasets(data_dir: Path | None = None) -> list[dict[str, Any]]:
    root = (data_dir or default_data_dir()).resolve()
    if not root.exists():
        return []

    return [
        {"filename": path.name, "extension": path.suffix.lower(), "size_bytes": path.stat().st_size}
        for path in sorted(root.iterdir(), key=lambda item: item.name.lower())
        if path.is_file() and path.suffix.lower() in SUPPORTED_EXTENSIONS
    ]


def resolve_dataset(filename: str, data_dir: Path | None = None) -> Path:
    root = (data_dir or default_data_dir()).resolve()
    candidate = (root / Path(filename).name).resolve()
    if not str(candidate).startswith(str(root)):
        raise ValueError("Dataset path is outside the configured data directory.")
    if candidate.suffix.lower() not in SUPPORTED_EXTENSIONS:
        raise ValueError(f"Unsupported dataset type: {candidate.suffix}")
    if not candidate.exists():
        raise FileNotFoundError(f"Dataset not found: {filename}")
    return candidate


def load_table(path: Path, sheet_name: str | None = None) -> pd.DataFrame:
    return load_table_with_metadata(path, sheet_name=sheet_name)["frame"]


def load_table_with_metadata(path: Path, sheet_name: str | None = None) -> dict[str, Any]:
    suffix = path.suffix.lower()
    if suffix in {".xlsx", ".xls"}:
        workbook = pd.ExcelFile(path)
        sheets = [str(name) for name in workbook.sheet_names]
        selected_sheet = sheet_name if sheet_name in sheets else sheets[0]
        raw = pd.read_excel(path, sheet_name=selected_sheet, header=None)
        frame, has_header, diagnostics = normalize_header_with_metadata(raw)
        return {
            "frame": frame,
            "source": {
                "encoding": None,
                "delimiter": None,
                "has_header": has_header,
                "header_row_index": diagnostics["header_row_index"],
                "dropped_empty_rows": diagnostics["dropped_empty_rows"],
                "dropped_empty_columns": diagnostics["dropped_empty_columns"],
                "sheets": sheets,
                "sheet_name": selected_sheet,
                "parser": "pandas.read_excel",
            },
        }
    if suffix in {".csv", ".txt"}:
        text, encoding = read_text_with_fallback(path)
        delimiter = detect_delimiter(text)
        raw = read_delimited_text(text, delimiter)
        frame, has_header, diagnostics = normalize_header_with_metadata(raw)
        return {
            "frame": frame,
            "source": {
                "encoding": encoding,
                "delimiter": delimiter_label(delimiter),
                "has_header": has_header,
                "header_row_index": diagnostics["header_row_index"],
                "dropped_empty_rows": diagnostics["dropped_empty_rows"],
                "dropped_empty_columns": diagnostics["dropped_empty_columns"],
                "sheets": [],
                "sheet_name": None,
                "parser": "pandas.read_csv",
            },
        }
    raise ValueError(f"Unsupported dataset type: {suffix}")


def read_text_with_fallback(path: Path) -> tuple[str, str]:
    last_error: Exception | None = None
    for encoding in TEXT_ENCODINGS:
        try:
            return path.read_text(encoding=encoding), encoding
        except UnicodeDecodeError as exc:
            last_error = exc
    if last_error:
        raise ValueError(f"Could not decode text file: {last_error}") from last_error
    raise ValueError("Could not decode text file.")


def detect_delimiter(text: str) -> str | None:
    sample = "\n".join(line for line in text.splitlines()[:20] if line.strip())
    if not sample:
        return ","
    try:
        dialect = csv.Sniffer().sniff(sample, delimiters=",\t;|")
        return dialect.delimiter
    except csv.Error:
        pass

    scores = {delimiter: sample.count(delimiter) for delimiter in [",", "\t", ";", "|"]}
    best, score = max(scores.items(), key=lambda item: item[1])
    if score > 0:
        return best
    return r"\s+"


def read_delimited_text(text: str, delimiter: str | None) -> pd.DataFrame:
    if delimiter == r"\s+":
        return pd.read_csv(io.StringIO(text), header=None, sep=delimiter, engine="python")
    return pd.read_csv(io.StringIO(text), header=None, sep=delimiter or ",", engine="python")


def delimiter_label(delimiter: str | None) -> str:
    if delimiter == "\t":
        return "tab"
    if delimiter == r"\s+":
        return "whitespace"
    return delimiter or "comma"


def normalize_header(raw: pd.DataFrame) -> tuple[pd.DataFrame, bool]:
    frame, has_header, _ = normalize_header_with_metadata(raw)
    return frame, has_header


def normalize_header_with_metadata(raw: pd.DataFrame) -> tuple[pd.DataFrame, bool, dict[str, Any]]:
    original_rows, original_columns = raw.shape
    raw = raw.replace(r"^\s*$", pd.NA, regex=True)
    raw = raw.dropna(axis=0, how="all").dropna(axis=1, how="all")
    diagnostics = {
        "header_row_index": None,
        "dropped_empty_rows": int(original_rows - raw.shape[0]),
        "dropped_empty_columns": int(original_columns - raw.shape[1]),
    }
    raw = raw.reset_index(drop=True)
    if raw.empty:
        return raw, False, diagnostics

    header_index = infer_header_row(raw)
    has_header = header_index is not None
    if has_header:
        diagnostics["header_row_index"] = header_index
        header = [safe_column_name(value, index) for index, value in enumerate(raw.iloc[header_index].tolist())]
        frame = raw.iloc[header_index + 1 :].reset_index(drop=True)
        frame.columns = make_unique(header)
        frame = remove_summary_rows(frame)
        return frame, True, diagnostics

    frame = raw.copy()
    frame.columns = [f"Column {index + 1}" for index in range(len(frame.columns))]
    frame = remove_summary_rows(frame)
    return frame.reset_index(drop=True), False, diagnostics


def infer_header(raw: pd.DataFrame) -> bool:
    return infer_header_row(raw) == 0


def infer_header_row(raw: pd.DataFrame, max_scan_rows: int = 15) -> int | None:
    if len(raw) < 2:
        return None
    best_index: int | None = None
    best_score = 0.0
    for index in range(min(max_scan_rows, len(raw) - 1)):
        candidate = raw.iloc[index]
        rest = raw.iloc[index + 1 :].head(20)
        non_empty = [value for value in candidate if not pd.isna(value) and str(value).strip()]
        if len(non_empty) < max(1, len(candidate) // 3):
            continue
        text_ratio = sum(not looks_numeric(value) for value in non_empty) / max(len(non_empty), 1)
        unique_ratio = len({str(value).strip() for value in non_empty}) / max(len(non_empty), 1)
        typed_ratio = min(1.0, numeric_ratio(rest) + date_ratio(rest))
        label_bonus = 0.2 if any(is_likely_header_label(str(value)) for value in non_empty) else 0.0
        score = text_ratio * 0.35 + unique_ratio * 0.25 + typed_ratio * 0.4 + label_bonus
        if score > best_score:
            best_score = score
            best_index = index
    return best_index if best_score >= 0.58 else None


def is_likely_header_label(text: str) -> bool:
    lowered = text.strip().lower()
    tokens = [
        "date", "time", "region", "channel", "product", "customer", "order",
        "revenue", "sales", "amount", "price", "cost", "profit", "rate",
        "日期", "时间", "地区", "渠道", "产品", "客户", "订单", "收入", "销售", "金额", "价格", "成本", "利润", "数量",
    ]
    return any(token in lowered for token in tokens)


def remove_summary_rows(frame: pd.DataFrame) -> pd.DataFrame:
    if frame.empty:
        return frame
    summary_pattern = re.compile(r"^(total|sum|subtotal|grand total|合计|总计|小计)$", re.IGNORECASE)
    keep = []
    for _, row in frame.iterrows():
        first_value = next((str(value).strip() for value in row.tolist() if not pd.isna(value) and str(value).strip()), "")
        keep.append(not bool(summary_pattern.match(first_value)))
    return frame.loc[keep].reset_index(drop=True)


def safe_column_name(value: Any, index: int) -> str:
    text = "" if pd.isna(value) else str(value).strip()
    return text or f"Column {index + 1}"


def make_unique(names: list[str]) -> list[str]:
    seen: dict[str, int] = {}
    unique = []
    for name in names:
        count = seen.get(name, 0)
        unique.append(name if count == 0 else f"{name}_{count + 1}")
        seen[name] = count + 1
    return unique


def looks_numeric(value: Any) -> bool:
    if pd.isna(value):
        return False
    try:
        float(str(value).replace(",", ""))
        return True
    except ValueError:
        return False


def numeric_ratio(df: pd.DataFrame) -> float:
    if df.empty:
        return 0.0
    total = max(df.shape[0] * df.shape[1], 1)
    converted = df.apply(pd.to_numeric, errors="coerce")
    return float(converted.notna().sum().sum()) / total


def date_ratio(df: pd.DataFrame) -> float:
    if df.empty:
        return 0.0
    total = max(df.shape[0] * df.shape[1], 1)
    converted = df.apply(parse_dates_quietly)
    return float(converted.notna().sum().sum()) / total


def parse_dates_quietly(series: pd.Series) -> pd.Series:
    with warnings.catch_warnings():
        warnings.simplefilter("ignore", UserWarning)
        return pd.to_datetime(series, errors="coerce")


def profile_dataset(filename: str, data_dir: Path | None = None, sheet_name: str | None = None) -> dict[str, Any]:
    path = resolve_dataset(filename, data_dir)
    loaded = load_table_with_metadata(path, sheet_name=sheet_name)
    return profile_table(path.name, path.suffix.lower(), loaded["frame"], loaded["source"])


def profile_uploaded_table(filename: str, df: pd.DataFrame) -> dict[str, Any]:
    suffix = Path(filename).suffix.lower()
    if suffix not in SUPPORTED_EXTENSIONS:
        raise ValueError(f"Unsupported dataset type: {suffix}")
    frame, has_header, diagnostics = normalize_header_with_metadata(df)
    source = {
        "encoding": None,
        "delimiter": None,
        "has_header": has_header,
        "header_row_index": diagnostics["header_row_index"],
        "dropped_empty_rows": diagnostics["dropped_empty_rows"],
        "dropped_empty_columns": diagnostics["dropped_empty_columns"],
        "sheets": [],
        "sheet_name": None,
        "parser": "upload",
    }
    return profile_table(Path(filename).name, suffix, frame, source)


def profile_table(filename: str, extension: str, df: pd.DataFrame, source: dict[str, Any] | None = None) -> dict[str, Any]:
    df = df.reset_index(drop=True)
    source_info = source or {}
    schema = infer_schema(df)
    numeric_columns = [item["name"] for item in schema if item["semantic_type"] == "numeric"]
    date_columns = [item["name"] for item in schema if item["semantic_type"] == "date"]
    category_columns = [item["name"] for item in schema if item["semantic_type"] == "category"]
    numeric_df = coerce_numeric_frame(df, numeric_columns)
    missing_cells = int(df.isna().sum().sum())

    columns = profile_columns(df, schema, numeric_df)
    anomalies = detect_anomalies(numeric_df)
    correlations = calculate_correlations(numeric_df)
    trends = detect_trends(numeric_df)
    chart_recommendations = recommend_charts(schema, numeric_columns, date_columns, category_columns)
    analysis_recommendations = recommend_analysis(schema, numeric_columns, date_columns, category_columns, anomalies)
    quality = score_data_quality(df, schema, numeric_df, missing_cells, anomalies)
    fingerprint = build_dataset_fingerprint(schema, quality)
    recommended_views = build_recommended_views(schema, numeric_columns, date_columns, category_columns, trends, correlations, anomalies)
    table_diagnostics = build_table_diagnostics(df, schema, source_info)
    quality_repair_plan = build_quality_repair_plan(df, schema, quality, anomalies, table_diagnostics)
    analysis_plan = build_analysis_plan(schema, quality, anomalies, trends, correlations, analysis_recommendations)
    insight_cards = build_insight_cards(
        df,
        quality,
        fingerprint,
        trends,
        correlations,
        anomalies,
        recommended_views,
    )
    executive_brief = build_executive_brief(
        df,
        schema,
        quality,
        anomalies,
        trends,
        analysis_recommendations,
        chart_recommendations,
        analysis_plan,
    )
    tool_trace = [
        "load_table",
        "detect_encoding",
        "detect_delimiter",
        "infer_header",
        "detect_sheet",
        "infer_schema",
        "profile_quality",
        "detect_anomalies",
        "recommend_analysis",
        "compose_insight",
    ]
    profile_id = build_profile_id(filename, source_info, df)
    generated_at = datetime.now(timezone.utc).isoformat()

    profile = {
        "session": {
            "id": profile_id,
            "generated_at": generated_at,
            "report_formats": ["markdown", "html"],
        },
        "dataset": {
            "filename": filename,
            "extension": extension,
            "rows": int(df.shape[0]),
            "columns": int(df.shape[1]),
            "numeric_columns": len(numeric_columns),
            "date_columns": len(date_columns),
            "category_columns": len(category_columns),
            "missing_cells": missing_cells,
        },
        "source": source_info,
        "table_diagnostics": table_diagnostics,
        "schema": schema,
        "quality": quality,
        "columns": columns,
        "anomalies": anomalies,
        "correlations": correlations,
        "trends": trends,
        "chart_recommendations": chart_recommendations,
        "recommended_views": recommended_views,
        "analysis_recommendations": analysis_recommendations,
        "analysis_plan": analysis_plan,
        "dataset_fingerprint": fingerprint,
        "insight_cards": insight_cards,
        "quality_repair_plan": quality_repair_plan,
        "executive_brief": executive_brief,
        "preview": build_table_preview(df),
        "tool_trace": tool_trace,
        "insights": build_insights(df, schema, missing_cells, anomalies, quality, trends, analysis_recommendations),
    }
    profile["local_ai"] = build_local_ai_enhancement(profile)
    return profile


def coerce_numeric_frame(df: pd.DataFrame, numeric_columns: list[str]) -> pd.DataFrame:
    numeric_df = pd.DataFrame()
    for column in numeric_columns:
        numeric_df[column] = pd.to_numeric(df[column], errors="coerce")
    return numeric_df


def infer_schema(df: pd.DataFrame) -> list[dict[str, Any]]:
    schema = []
    row_count = max(len(df), 1)
    for column in df.columns:
        series = df[column]
        missing = int(series.isna().sum())
        non_null = series.dropna()
        numeric = pd.to_numeric(series, errors="coerce")
        numeric_valid = int(numeric.notna().sum())
        date = parse_dates_quietly(series)
        date_valid = int(date.notna().sum())
        unique_count = int(non_null.astype(str).nunique()) if not non_null.empty else 0
        unique_ratio = unique_count / max(len(non_null), 1)

        semantic_type = "empty"
        if numeric_valid / row_count >= 0.8:
            semantic_type = "numeric"
        elif date_valid / row_count >= 0.7:
            semantic_type = "date"
        elif unique_count <= max(20, row_count * 0.2):
            semantic_type = "category"
        elif non_null.astype(str).str.len().mean() > 30:
            semantic_type = "text"
        else:
            semantic_type = "category" if unique_ratio < 0.8 else "high_cardinality"

        schema.append(
            {
                "name": str(column),
                "semantic_type": semantic_type,
                "missing_count": missing,
                "missing_ratio": _safe_float(missing / row_count),
                "unique_count": unique_count,
                "unique_ratio": _safe_float(unique_ratio),
                "is_analyzable": semantic_type in {"numeric", "date", "category"},
                "role_hint": role_hint(str(column), semantic_type),
            }
        )
    return schema


def role_hint(name: str, semantic_type: str) -> str:
    lowered = name.lower()
    if semantic_type == "date" or any(token in lowered for token in ["date", "time", "日期", "时间"]):
        return "time_axis"
    if any(token in lowered for token in ["id", "编号", "编码", "code", "订单号", "客户号"]):
        return "identifier"
    if any(token in lowered for token in ["target", "label", "标签", "目标", "结果"]):
        return "target"
    if any(token in lowered for token in ["revenue", "sales", "amount", "price", "cost", "profit", "收入", "销售", "金额", "价格", "成本", "利润"]):
        return "business_measure"
    if semantic_type == "numeric":
        return "measure"
    if semantic_type == "category":
        return "dimension"
    return "metadata"


def profile_columns(df: pd.DataFrame, schema: list[dict[str, Any]], numeric_df: pd.DataFrame) -> list[dict[str, Any]]:
    profiles = []
    for item in schema:
        name = item["name"]
        profile = {"column": name, **item}
        if item["semantic_type"] == "numeric" and name in numeric_df:
            series = numeric_df[name].dropna()
            profile.update(
                {
                    "count": int(series.count()),
                    "mean": _safe_float(series.mean()),
                    "std": _safe_float(series.std(ddof=0)),
                    "min": _safe_float(series.min()),
                    "median": _safe_float(series.median()),
                    "max": _safe_float(series.max()),
                }
            )
        profiles.append(profile)
    return profiles


def detect_anomalies(numeric_df: pd.DataFrame, threshold: float = 2.5) -> list[dict[str, Any]]:
    results = []
    for column in numeric_df.columns:
        series = numeric_df[column].dropna()
        if len(series) < 3:
            continue
        std = series.std(ddof=0)
        if not std or math.isnan(std):
            continue
        mean = series.mean()
        z_scores = ((series - mean) / std).abs()
        for row_index, score in z_scores[z_scores >= threshold].sort_values(ascending=False).items():
            results.append(
                {
                    "row": int(row_index),
                    "column": str(column),
                    "value": _safe_float(numeric_df.at[row_index, column]),
                    "z_score": _safe_float(score),
                }
            )
    return results[:20]


def calculate_correlations(numeric_df: pd.DataFrame, limit: int = 10) -> list[dict[str, Any]]:
    if len(numeric_df.columns) < 2:
        return []
    corr = numeric_df.corr(numeric_only=True)
    pairs = []
    columns = list(corr.columns)
    for i, left in enumerate(columns):
        for right in columns[i + 1 :]:
            value = corr.at[left, right]
            if pd.isna(value):
                continue
            pairs.append(
                {
                    "left": str(left),
                    "right": str(right),
                    "correlation": _safe_float(value),
                    "strength": correlation_strength(float(value)),
                }
            )
    return sorted(pairs, key=lambda item: abs(item["correlation"] or 0), reverse=True)[:limit]


def detect_trends(numeric_df: pd.DataFrame) -> list[dict[str, Any]]:
    trends = []
    for column in numeric_df.columns:
        series = numeric_df[column].dropna().reset_index(drop=True)
        if len(series) < 3:
            continue
        first = float(series.iloc[0])
        last = float(series.iloc[-1])
        slope = (last - first) / max(len(series) - 1, 1)
        direction = "flat"
        if slope > 0:
            direction = "up"
        elif slope < 0:
            direction = "down"
        trends.append(
            {
                "column": str(column),
                "direction": direction,
                "slope": _safe_float(slope),
                "first": _safe_float(first),
                "last": _safe_float(last),
            }
        )
    return sorted(trends, key=lambda item: abs(item["slope"] or 0), reverse=True)


def score_data_quality(
    raw_df: pd.DataFrame,
    schema: list[dict[str, Any]],
    numeric_df: pd.DataFrame,
    missing_cells: int,
    anomalies: list[dict[str, Any]],
) -> dict[str, Any]:
    total_cells = max(int(raw_df.shape[0] * raw_df.shape[1]), 1)
    analyzable_count = sum(1 for item in schema if item["is_analyzable"])
    numeric_ratio_value = len(numeric_df.columns) / max(raw_df.shape[1], 1)
    analyzable_ratio = analyzable_count / max(raw_df.shape[1], 1)
    missing_ratio_value = missing_cells / total_cells
    duplicate_count = int(raw_df.duplicated().sum())
    sample_penalty = 15 if len(raw_df) < 5 else 5 if len(raw_df) < 20 else 0
    score = 100
    score -= min(35, missing_ratio_value * 100)
    score -= min(20, (duplicate_count / max(len(raw_df), 1)) * 100)
    score -= min(20, len(anomalies) * 2)
    score -= sample_penalty
    score -= max(0, 20 - analyzable_ratio * 20)
    score = round(max(0, min(100, score)))
    level = "high" if score >= 80 else "medium" if score >= 60 else "low"
    return {
        "score": score,
        "level": level,
        "numeric_ratio": _safe_float(numeric_ratio_value),
        "analyzable_ratio": _safe_float(analyzable_ratio),
        "missing_ratio": _safe_float(missing_ratio_value),
        "duplicate_rows": duplicate_count,
        "anomaly_count": len(anomalies),
        "sample_warning": len(raw_df) < 20,
    }


def build_profile_id(filename: str, source: dict[str, Any], df: pd.DataFrame) -> str:
    digest = sha256()
    digest.update(filename.encode("utf-8", errors="ignore"))
    digest.update(str(source.get("sheet_name") or "").encode("utf-8", errors="ignore"))
    digest.update(str(df.shape).encode("utf-8"))
    digest.update("|".join(str(column) for column in df.columns).encode("utf-8", errors="ignore"))
    return digest.hexdigest()[:16]


def build_table_diagnostics(df: pd.DataFrame, schema: list[dict[str, Any]], source: dict[str, Any]) -> dict[str, Any]:
    empty_columns = [item["name"] for item in schema if item["semantic_type"] == "empty"]
    duplicate_columns = find_duplicate_columns(df)
    likely_identifier_columns = [item["name"] for item in schema if item["role_hint"] == "identifier"]
    high_cardinality_columns = [item["name"] for item in schema if item["semantic_type"] == "high_cardinality"]
    total_like_rows = count_total_like_rows(df)
    return {
        "header_row_index": source.get("header_row_index"),
        "has_header": source.get("has_header", False),
        "encoding": source.get("encoding"),
        "delimiter": source.get("delimiter"),
        "sheet_name": source.get("sheet_name"),
        "sheets": source.get("sheets", []),
        "dropped_empty_rows": source.get("dropped_empty_rows", 0),
        "dropped_empty_columns": source.get("dropped_empty_columns", 0),
        "empty_columns": empty_columns,
        "duplicate_columns": duplicate_columns,
        "likely_identifier_columns": likely_identifier_columns,
        "high_cardinality_columns": high_cardinality_columns,
        "total_like_rows": total_like_rows,
        "messy_score": calculate_messy_score(source, empty_columns, duplicate_columns, high_cardinality_columns, total_like_rows),
    }


def find_duplicate_columns(df: pd.DataFrame) -> list[dict[str, str]]:
    duplicates: list[dict[str, str]] = []
    columns = list(df.columns)
    for left_index, left in enumerate(columns):
        left_series = df[left].astype(str).fillna("")
        for right in columns[left_index + 1 :]:
            right_series = df[right].astype(str).fillna("")
            if left_series.equals(right_series):
                duplicates.append({"left": str(left), "right": str(right)})
    return duplicates[:10]


def count_total_like_rows(df: pd.DataFrame) -> int:
    pattern = re.compile(r"^(total|sum|subtotal|grand total|合计|总计|小计)$", re.IGNORECASE)
    count = 0
    for _, row in df.iterrows():
        first_value = next((str(value).strip() for value in row.tolist() if not pd.isna(value) and str(value).strip()), "")
        if pattern.match(first_value):
            count += 1
    return count


def calculate_messy_score(
    source: dict[str, Any],
    empty_columns: list[str],
    duplicate_columns: list[dict[str, str]],
    high_cardinality_columns: list[str],
    total_like_rows: int,
) -> int:
    score = 0
    score += 15 if not source.get("has_header") else 0
    score += min(20, int(source.get("dropped_empty_rows") or 0) * 5)
    score += min(20, int(source.get("dropped_empty_columns") or 0) * 5)
    score += min(20, len(empty_columns) * 5)
    score += min(20, len(duplicate_columns) * 10)
    score += min(15, len(high_cardinality_columns) * 3)
    score += min(10, total_like_rows * 5)
    return min(score, 100)


def schema_roles(schema: list[dict[str, Any]]) -> dict[str, list[str]]:
    return {
        "time_axis": [item["name"] for item in schema if item["role_hint"] == "time_axis"],
        "dimensions": [item["name"] for item in schema if item["role_hint"] == "dimension"],
        "measures": [item["name"] for item in schema if item["role_hint"] in {"measure", "business_measure"}],
        "business_measures": [item["name"] for item in schema if item["role_hint"] == "business_measure"],
        "identifiers": [item["name"] for item in schema if item["role_hint"] == "identifier"],
        "text_fields": [item["name"] for item in schema if item["semantic_type"] == "text"],
    }


def build_dataset_fingerprint(schema: list[dict[str, Any]], quality: dict[str, Any]) -> dict[str, Any]:
    roles = schema_roles(schema)
    names = " ".join(item["name"].lower() for item in schema)
    dataset_type = "generic_table"
    label = "Generic tabular dataset"
    if any(token in names for token in ["sales", "revenue", "order", "customer", "channel", "product", "销售", "收入", "订单", "客户"]):
        dataset_type = "sales_operations"
        label = "Sales or operations table"
    elif any(token in names for token in ["price", "cost", "profit", "amount", "finance", "金额", "价格", "成本", "利润"]):
        dataset_type = "finance_transaction"
        label = "Financial or transaction table"
    elif roles["time_axis"] and roles["measures"]:
        dataset_type = "time_series"
        label = "Time-series measurement table"
    elif roles["dimensions"] and roles["measures"]:
        dataset_type = "dimensional_analysis"
        label = "Dimensional analysis table"

    confidence_score = 45
    confidence_score += 20 if roles["measures"] else 0
    confidence_score += 15 if roles["dimensions"] else 0
    confidence_score += 15 if roles["time_axis"] else 0
    confidence_score += 5 if quality["score"] >= 80 else 0
    confidence_score = min(confidence_score, 95)
    confidence = "high" if confidence_score >= 80 else "medium" if confidence_score >= 60 else "low"

    summary = (
        f"{label} with {len(roles['measures'])} measure fields, "
        f"{len(roles['dimensions'])} dimensions, and {len(roles['time_axis'])} time axes."
    )
    if quality.get("sample_warning"):
        summary += " Sample size is limited, so conclusions should stay exploratory."

    return {
        "type": dataset_type,
        "label": label,
        "confidence": confidence,
        "confidence_score": confidence_score,
        "summary": summary,
        "roles": roles,
        "field_counts": {
            "measures": len(roles["measures"]),
            "dimensions": len(roles["dimensions"]),
            "time_axis": len(roles["time_axis"]),
            "identifiers": len(roles["identifiers"]),
            "text_fields": len(roles["text_fields"]),
        },
        "primary_time_axis": roles["time_axis"][0] if roles["time_axis"] else None,
        "primary_measure": roles["business_measures"][0] if roles["business_measures"] else (roles["measures"][0] if roles["measures"] else None),
        "primary_dimension": roles["dimensions"][0] if roles["dimensions"] else None,
    }


def build_recommended_views(
    schema: list[dict[str, Any]],
    numeric_columns: list[str],
    date_columns: list[str],
    category_columns: list[str],
    trends: list[dict[str, Any]],
    correlations: list[dict[str, Any]],
    anomalies: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    views: list[dict[str, Any]] = []
    preferred_measure = preferred_metric(schema, numeric_columns)
    if date_columns and numeric_columns:
        views.append(
            {
                "id": "trend",
                "label": "Trend",
                "chart_type": "line",
                "x": date_columns[0],
                "y": ",".join(top_metrics(schema, numeric_columns, limit=3)),
                "reason": "A time field and numeric measures were detected.",
                "priority": 1,
                "enabled": True,
            }
        )
    elif len(numeric_columns) >= 2:
        views.append(
            {
                "id": "trend",
                "label": "Trend",
                "chart_type": "line",
                "x": "record_index",
                "y": ",".join(top_metrics(schema, numeric_columns, limit=3)),
                "reason": "No date field was detected, so record order is used for exploratory trend review.",
                "priority": 2,
                "enabled": True,
            }
        )
    if category_columns and preferred_measure:
        views.append(
            {
                "id": "segment",
                "label": "Segment",
                "chart_type": "bar",
                "x": category_columns[0],
                "y": preferred_measure,
                "reason": "A categorical dimension can explain differences in a key metric.",
                "priority": 2,
                "enabled": True,
            }
        )
    if len(numeric_columns) >= 2 and correlations:
        views.append(
            {
                "id": "correlation",
                "label": "Correlation",
                "chart_type": "heatmap",
                "x": "numeric_fields",
                "y": "numeric_fields",
                "reason": "Multiple numeric fields are available for relationship review.",
                "priority": 3,
                "enabled": True,
            }
        )
    if preferred_measure:
        views.append(
            {
                "id": "distribution",
                "label": "Distribution",
                "chart_type": "bar",
                "x": "record_index",
                "y": preferred_measure,
                "reason": "A selected metric can be reviewed for spread and outliers.",
                "priority": 4,
                "enabled": True,
            }
        )
    views.append(
        {
            "id": "quality",
            "label": "Quality",
            "chart_type": "table_review",
            "x": "",
            "y": "",
            "reason": "Quality review keeps missing values, duplicates, and anomalies visible before interpretation.",
            "priority": 5 if not anomalies else 1,
            "enabled": True,
        }
    )
    return sorted(views, key=lambda item: item["priority"])


def preferred_metric(schema: list[dict[str, Any]], numeric_columns: list[str]) -> str | None:
    if not numeric_columns:
        return None
    business = [item["name"] for item in schema if item["name"] in numeric_columns and item["role_hint"] == "business_measure"]
    if business:
        return business[0]
    scored = sorted(numeric_columns, key=lambda name: metric_priority(name), reverse=True)
    return scored[0]


def top_metrics(schema: list[dict[str, Any]], numeric_columns: list[str], limit: int = 3) -> list[str]:
    if not numeric_columns:
        return []
    preferred = preferred_metric(schema, numeric_columns)
    ordered = [preferred] if preferred else []
    for name in sorted(numeric_columns, key=lambda item: metric_priority(item), reverse=True):
        if name not in ordered:
            ordered.append(name)
    return ordered[:limit]


def metric_priority(name: str) -> int:
    lowered = name.lower()
    score = 0
    for token in ["revenue", "sales", "profit", "amount", "income", "收入", "销售", "利润", "金额"]:
        if token in lowered:
            score += 20
    for token in ["order", "count", "qty", "volume", "订单", "数量"]:
        if token in lowered:
            score += 10
    for token in ["rate", "ratio", "margin", "率", "比例"]:
        if token in lowered:
            score += 5
    return score


def build_quality_repair_plan(
    df: pd.DataFrame,
    schema: list[dict[str, Any]],
    quality: dict[str, Any],
    anomalies: list[dict[str, Any]],
    diagnostics: dict[str, Any] | None = None,
) -> list[dict[str, Any]]:
    plan: list[dict[str, Any]] = []
    diagnostics = diagnostics or {}
    if diagnostics.get("dropped_empty_rows") or diagnostics.get("dropped_empty_columns"):
        plan.append(
            {
                "id": "empty_structure",
                "severity": "low",
                "title": "Confirm removed empty rows or columns",
                "evidence": (
                    f"Dropped {diagnostics.get('dropped_empty_rows', 0)} empty rows and "
                    f"{diagnostics.get('dropped_empty_columns', 0)} empty columns during parsing."
                ),
                "recommendation": "Keep the cleaned table for analysis, but confirm the removed areas did not contain notes or hidden headers.",
                "auto_fix": True,
                "impact": "The active dataset is already analyzed without fully empty rows and columns.",
            }
        )
    if diagnostics.get("duplicate_columns"):
        duplicate = diagnostics["duplicate_columns"][0]
        plan.append(
            {
                "id": "duplicate_columns",
                "severity": "medium",
                "title": f"Review duplicate fields {duplicate['left']} and {duplicate['right']}",
                "evidence": "Two columns carry identical values in the loaded preview.",
                "recommendation": "Merge or remove duplicated fields before building a final report.",
                "auto_fix": False,
                "impact": "Leaving duplicate fields can overstate schema richness and correlation evidence.",
            }
        )
    missing_fields = [item for item in schema if item["missing_count"] > 0]
    if missing_fields:
        top = sorted(missing_fields, key=lambda item: item["missing_ratio"], reverse=True)[0]
        plan.append(
            {
                "id": "missing_values",
                "severity": "high" if top["missing_ratio"] > 0.2 else "medium",
                "title": f"Review missing values in {top['name']}",
                "evidence": f"{top['missing_count']} missing cells, ratio {top['missing_ratio']}.",
                "recommendation": "Decide whether to fill, exclude, or keep missing values before charting.",
                "auto_fix": top["semantic_type"] in {"numeric", "category"},
                "impact": f"About {round((top['missing_ratio'] or 0) * 100, 2)}% of this field may be affected.",
            }
        )
    duplicates = int(df.duplicated().sum())
    if duplicates:
        plan.append(
            {
                "id": "duplicate_rows",
                "severity": "medium",
                "title": "Review duplicate records",
                "evidence": f"{duplicates} duplicate rows detected.",
                "recommendation": "Confirm whether duplicates are legitimate repeated events or import errors.",
                "auto_fix": False,
                "impact": f"{duplicates} rows may inflate counts or sums if they are accidental duplicates.",
            }
        )
    if anomalies:
        top = anomalies[0]
        plan.append(
            {
                "id": "anomaly_review",
                "severity": "medium",
                "title": f"Review high-deviation values in {top['column']}",
                "evidence": f"Top z-score is {top['z_score']} at row {top['row'] + 1}.",
                "recommendation": "Inspect highlighted values before using them in summaries.",
                "auto_fix": False,
                "impact": "Outliers can change averages, trend slopes, and chart scaling.",
            }
        )
    if quality.get("sample_warning"):
        plan.append(
            {
                "id": "sample_size",
                "severity": "low",
                "title": "Treat results as exploratory",
                "evidence": f"Only {len(df)} rows are available.",
                "recommendation": "Use the report for directional review, not final statistical conclusions.",
                "auto_fix": False,
                "impact": "Small samples reduce confidence in trend, correlation, and anomaly findings.",
            }
        )
    if not plan:
        plan.append(
            {
                "id": "quality_clear",
                "severity": "info",
                "title": "No blocking data quality issue detected",
                "evidence": "Missing values, duplicates, and high z-score anomalies are within acceptable limits.",
                "recommendation": "Proceed to trend, segment, and relationship analysis.",
                "auto_fix": False,
                "impact": "No immediate cleaning step is required before exploratory analysis.",
            }
        )
    return plan


def build_insight_cards(
    df: pd.DataFrame,
    quality: dict[str, Any],
    fingerprint: dict[str, Any],
    trends: list[dict[str, Any]],
    correlations: list[dict[str, Any]],
    anomalies: list[dict[str, Any]],
    recommended_views: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    cards: list[dict[str, Any]] = [
        {
            "id": "dataset_fingerprint",
            "title": fingerprint["label"],
            "summary": fingerprint["summary"],
            "evidence": f"{df.shape[0]} rows, {df.shape[1]} columns, confidence {fingerprint['confidence']}.",
            "confidence": fingerprint["confidence"],
            "severity": "info",
            "action": {"label": "Review data structure", "view": "quality"},
        },
        {
            "id": "quality_score",
            "title": f"Data quality is {quality['level']}",
            "summary": f"Quality score is {quality['score']}/100 with {quality['anomaly_count']} anomaly candidates.",
            "evidence": f"Missing ratio {quality['missing_ratio']}, duplicate rows {quality['duplicate_rows']}.",
            "confidence": "high",
            "severity": "positive" if quality["score"] >= 80 else "warning",
            "action": {"label": "Review quality", "view": "quality"},
        },
    ]
    if trends:
        trend = trends[0]
        cards.append(
            {
                "id": "top_trend",
                "title": f"{trend['column']} is moving {trend['direction']}",
                "summary": f"The strongest simple trend is {trend['column']} with slope {trend['slope']}.",
                "evidence": f"First value {trend['first']}, last value {trend['last']}.",
                "confidence": "medium",
                "severity": "info",
                "action": {"label": "Open trend", "view": "trend"},
            }
        )
    segment_view = next((view for view in recommended_views if view["id"] == "segment"), None)
    if segment_view:
        cards.append(
            {
                "id": "segment_opportunity",
                "title": f"Compare {segment_view['y']} by {segment_view['x']}",
                "summary": "A category field and a key measure can be used for segment comparison.",
                "evidence": f"Recommended view uses {segment_view['chart_type']} with x={segment_view['x']} and y={segment_view['y']}.",
                "confidence": "medium",
                "severity": "info",
                "action": {"label": "Open segment view", "view": "segment"},
            }
        )
    if correlations:
        corr = correlations[0]
        cards.append(
            {
                "id": "top_correlation",
                "title": f"{corr['left']} and {corr['right']} are {corr['strength']}ly related",
                "summary": f"Correlation is {corr['correlation']}. Review whether this relationship is expected.",
                "evidence": f"Calculated from available numeric records for {corr['left']} and {corr['right']}.",
                "confidence": "medium",
                "severity": "info",
                "action": {"label": "Review relationship", "view": "correlation"},
            }
        )
    if anomalies:
        cards.append(
            {
                "id": "anomaly_review",
                "title": "Some values need review",
                "summary": f"{len(anomalies)} high-deviation cells were detected.",
                "evidence": f"Highest candidate: row {anomalies[0]['row'] + 1}, {anomalies[0]['column']} = {anomalies[0]['value']}.",
                "confidence": "medium",
                "severity": "warning",
                "action": {"label": "Inspect quality", "view": "quality"},
            }
        )
    elif any(view["id"] == "distribution" for view in recommended_views):
        cards.append(
            {
                "id": "distribution_ready",
                "title": "Distribution review is available",
                "summary": "A numeric metric can be inspected across records to understand spread.",
                "evidence": "At least one analyzable numeric measure was detected.",
                "confidence": "high",
                "severity": "info",
                "action": {"label": "Open distribution", "view": "distribution"},
            }
        )
    return cards[:6]


def build_local_ai_enhancement(profile: dict[str, Any]) -> dict[str, Any]:
    enabled = any(
        os.getenv(name, "").lower() in {"1", "true", "yes"}
        for name in ["TABLEPILOT_ENABLE_LOCAL_AI", "TABLEPILOT_ENABLE_OLLAMA"]
    )
    provider = os.getenv("TABLEPILOT_LOCAL_AI_PROVIDER", "").lower().strip()
    base_url = os.getenv("LOCAL_LLM_BASE_URL", "").rstrip("/")
    if not provider:
        provider = "openai-compatible" if base_url else "ollama"
    model = os.getenv("LOCAL_LLM_MODEL") or os.getenv("OLLAMA_MODEL", "qwen2.5:1.5b")
    if not enabled:
        return {
            "provider": provider,
            "model": model,
            "status": "disabled",
            "summary": None,
            "guardrail": "Local AI wording is disabled; deterministic evidence is used.",
        }

    prompt = build_ollama_prompt(profile)
    if provider in {"openai-compatible", "llama", "llamacpp", "lmstudio"}:
        return call_openai_compatible_completion(prompt, model, base_url or "http://127.0.0.1:39281/v1", profile)

    endpoint = os.getenv("OLLAMA_URL", "http://127.0.0.1:11434/api/generate")
    payload = json.dumps({"model": model, "prompt": prompt, "stream": False}).encode("utf-8")
    request = urllib.request.Request(endpoint, data=payload, headers={"Content-Type": "application/json"}, method="POST")
    try:
        with urllib.request.urlopen(request, timeout=8) as response:
            body = json.loads(response.read().decode("utf-8"))
    except (urllib.error.URLError, TimeoutError, json.JSONDecodeError, OSError) as exc:
        return {
            "provider": "ollama",
            "model": model,
            "status": "unavailable",
            "summary": None,
            "error": str(exc),
            "guardrail": "Ollama was requested but not available; deterministic evidence remains authoritative.",
        }

    response_text = str(body.get("response", "")).strip()
    guardrail = validate_local_ai_summary(response_text, profile)
    if not guardrail["ok"]:
        return {
            "provider": "ollama",
            "model": model,
            "status": "guardrail_failed",
            "summary": None,
            "error": guardrail["reason"],
            "guardrail": "The local model output was suppressed because it referenced unsupported evidence.",
        }
    return {
        "provider": "ollama",
        "model": model,
        "status": "generated" if response_text else "empty",
        "summary": response_text or None,
        "guardrail": "The local model can only rewrite the structured evidence; it must not add unsupported conclusions.",
    }


def call_openai_compatible_completion(prompt: str, model: str, base_url: str, profile: dict[str, Any]) -> dict[str, Any]:
    endpoint = f"{base_url.rstrip('/')}/completions"
    payload = json.dumps(
        {
            "model": model,
            "prompt": prompt,
            "max_tokens": int(os.getenv("LOCAL_LLM_MAX_TOKENS", "220")),
            "temperature": float(os.getenv("LOCAL_LLM_TEMPERATURE", "0.2")),
        }
    ).encode("utf-8")
    request = urllib.request.Request(endpoint, data=payload, headers={"Content-Type": "application/json"}, method="POST")
    try:
        with urllib.request.urlopen(request, timeout=20) as response:
            body = json.loads(response.read().decode("utf-8"))
    except (urllib.error.URLError, TimeoutError, json.JSONDecodeError, OSError) as exc:
        return {
            "provider": "openai-compatible",
            "model": model,
            "status": "unavailable",
            "summary": None,
            "error": str(exc),
            "guardrail": "The local /v1 completion endpoint was requested but not available; deterministic evidence remains authoritative.",
        }

    choices = body.get("choices", [])
    response_text = ""
    if choices:
        first = choices[0]
        response_text = str(first.get("text") or first.get("message", {}).get("content") or "").strip()
    guardrail = validate_local_ai_summary(response_text, profile)
    if not guardrail["ok"]:
        return {
            "provider": "openai-compatible",
            "model": model,
            "status": "guardrail_failed",
            "summary": None,
            "error": guardrail["reason"],
            "usage": body.get("usage", {}),
            "guardrail": "The local model output was suppressed because it referenced unsupported evidence.",
        }
    return {
        "provider": "openai-compatible",
        "model": model,
        "status": "generated" if response_text else "empty",
        "summary": response_text or None,
        "usage": body.get("usage", {}),
        "guardrail": "The local model can only rewrite the structured evidence; it must not add unsupported conclusions.",
    }


def validate_local_ai_summary(summary: str, profile: dict[str, Any]) -> dict[str, Any]:
    if not summary:
        return {"ok": True, "reason": ""}
    allowed_fields = {str(item["name"]).lower() for item in profile.get("schema", [])}
    referenced_metric_tokens = {token.lower() for token in re.findall(r"\bmetric_[a-zA-Z0-9_]+\b", summary)}
    unsupported = sorted(token for token in referenced_metric_tokens if token not in allowed_fields)
    if unsupported:
        return {"ok": False, "reason": f"Unsupported field reference(s): {', '.join(unsupported)}"}
    return {"ok": True, "reason": ""}


def build_ollama_prompt(profile: dict[str, Any]) -> str:
    dataset = profile["dataset"]
    quality = profile["quality"]
    cards = profile.get("insight_cards", [])[:4]
    evidence_lines = [
        f"Dataset: {dataset['filename']}, rows={dataset['rows']}, columns={dataset['columns']}",
        f"Quality: {quality['score']}/100, missing={dataset['missing_cells']}, anomalies={quality['anomaly_count']}",
    ]
    for card in cards:
        evidence_lines.append(f"- {card['title']}: {card['summary']} Evidence: {card['evidence']}")
    return (
        "You are TablePilot, a local evidence-grounded data analysis assistant. "
        "Write a concise business analysis summary in English only. "
        "Do not add translation, do not invent facts, and do not mention fields beyond the evidence below.\n\n"
        + "\n".join(evidence_lines)
    )


def recommend_analysis(
    schema: list[dict[str, Any]],
    numeric_columns: list[str],
    date_columns: list[str],
    category_columns: list[str],
    anomalies: list[dict[str, Any]],
) -> list[dict[str, str]]:
    recommendations: list[dict[str, str]] = []
    if date_columns and numeric_columns:
        recommendations.append(
            {
                "type": "trend",
                "title": f"Analyze {numeric_columns[0]} over {date_columns[0]}",
                "reason": "A date column and numeric measure were detected.",
            }
        )
    if category_columns and numeric_columns:
        recommendations.append(
            {
                "type": "group_compare",
                "title": f"Compare {numeric_columns[0]} by {category_columns[0]}",
                "reason": "Categorical dimensions can explain differences in numeric measures.",
            }
        )
    if len(numeric_columns) >= 2:
        recommendations.append(
            {
                "type": "correlation",
                "title": "Review relationships between numeric columns",
                "reason": "Multiple numeric columns are available for correlation analysis.",
            }
        )
    if anomalies:
        recommendations.append(
            {
                "type": "anomaly_review",
                "title": "Review high-deviation data points",
                "reason": "High z-score cells were detected.",
            }
        )
    high_missing = [item["name"] for item in schema if (item["missing_ratio"] or 0) > 0.1]
    if high_missing:
        recommendations.append(
            {
                "type": "quality",
                "title": f"Investigate missing values in {high_missing[0]}",
                "reason": "At least one column has a missing ratio above 10%.",
            }
        )
    high_cardinality = [item["name"] for item in schema if item["semantic_type"] == "high_cardinality"]
    if high_cardinality:
        recommendations.append(
            {
                "type": "field_review",
                "title": f"Treat {high_cardinality[0]} as identifier or metadata",
                "reason": "High-cardinality fields are usually not useful as grouping dimensions.",
            }
        )
    return recommendations[:6]


def recommend_charts(
    schema: list[dict[str, Any]],
    numeric_columns: list[str],
    date_columns: list[str],
    category_columns: list[str],
) -> list[dict[str, str]]:
    if date_columns and numeric_columns:
        return [
            {
                "chart_type": "line",
                "x": date_columns[0],
                "y": ",".join(numeric_columns[:5]),
                "reason": "Best for showing measures over time.",
            }
        ]
    if category_columns and numeric_columns:
        return [
            {
                "chart_type": "bar",
                "x": category_columns[0],
                "y": numeric_columns[0],
                "reason": "Best for comparing a measure across categories.",
            }
        ]
    if len(numeric_columns) >= 2:
        return [
            {
                "chart_type": "line",
                "x": "row_index",
                "y": ",".join(numeric_columns[:5]),
                "reason": "No date column was found, so row index is used as the x-axis.",
            }
        ]
    if numeric_columns:
        return [
            {
                "chart_type": "bar",
                "x": "row_index",
                "y": numeric_columns[0],
                "reason": "A single numeric column is available.",
            }
        ]
    return [{"chart_type": "none", "x": "", "y": "", "reason": "No reliable numeric column was detected."}]


def correlation_strength(value: float) -> str:
    magnitude = abs(value)
    if magnitude >= 0.8:
        return "strong"
    if magnitude >= 0.5:
        return "moderate"
    return "weak"


def build_table_preview(df: pd.DataFrame, max_rows: int = 500) -> dict[str, Any]:
    preview = df.head(max_rows)
    rows = [[to_json_cell(value) for value in row.tolist()] for _, row in preview.iterrows()]
    return {
        "columns": [str(column) for column in df.columns],
        "rows": rows,
        "returned_rows": len(rows),
        "max_rows": max_rows,
    }


def to_json_cell(value: Any) -> Any:
    if pd.isna(value):
        return ""
    if isinstance(value, pd.Timestamp):
        return value.isoformat()
    if hasattr(value, "item"):
        value = value.item()
    if isinstance(value, float):
        return _safe_float(value)
    return value


def build_insights(
    raw_df: pd.DataFrame,
    schema: list[dict[str, Any]],
    missing_cells: int,
    anomalies: list[dict[str, Any]],
    quality: dict[str, Any],
    trends: list[dict[str, Any]],
    recommendations: list[dict[str, str]],
) -> list[str]:
    insights = [
        f"Loaded {raw_df.shape[0]} rows and {raw_df.shape[1]} columns.",
        f"Detected {sum(1 for item in schema if item['semantic_type'] == 'numeric')} numeric columns, "
        f"{sum(1 for item in schema if item['semantic_type'] == 'date')} date columns, and "
        f"{sum(1 for item in schema if item['semantic_type'] == 'category')} categorical columns.",
        f"Data quality is {quality['level']} with a score of {quality['score']}/100.",
    ]
    if missing_cells:
        insights.append(f"Found {missing_cells} missing cells; downstream analysis should account for incomplete data.")
    else:
        insights.append("No missing cells were detected in the loaded table.")
    if anomalies:
        insights.append(f"Detected {len(anomalies)} high z-score cells for review.")
    else:
        insights.append("No high z-score anomalies were detected with the default threshold.")
    if trends:
        strongest = trends[0]
        insights.append(f"The strongest simple trend is column {strongest['column']} moving {strongest['direction']}.")
    if recommendations:
        insights.append(f"Recommended next analysis: {recommendations[0]['title']}.")
    return insights


def build_executive_brief(
    df: pd.DataFrame,
    schema: list[dict[str, Any]],
    quality: dict[str, Any],
    anomalies: list[dict[str, Any]],
    trends: list[dict[str, Any]],
    recommendations: list[dict[str, Any]],
    chart_recommendations: list[dict[str, Any]],
    analysis_plan: dict[str, Any],
) -> dict[str, Any]:
    analyzable_fields = [item for item in schema if item["is_analyzable"]]
    headline = (
        f"{analysis_plan['dataset_story']} Loaded {len(df)} rows and {len(df.columns)} columns with "
        f"{len(analyzable_fields)} analyzable fields."
    )
    if trends:
        strongest = trends[0]
        headline += f" Strongest trend: {strongest['column']} moving {strongest['direction']}."
    elif anomalies:
        headline += f" {len(anomalies)} values need anomaly review."

    confidence = "high" if quality["score"] >= 80 else "medium" if quality["score"] >= 60 else "low"
    watchouts = []
    if quality["missing_ratio"] > 0:
        watchouts.append("Missing values may bias aggregates and charts.")
    if anomalies:
        watchouts.append("High z-score values should be reviewed before drawing conclusions.")
    if len(df) < 10:
        watchouts.append("Small sample size limits statistical confidence.")
    if not watchouts:
        watchouts.append("No major structural data quality warning was detected.")

    next_moves = [item["title"] for item in recommendations[:3]]
    next_moves.extend(step["title"] for step in analysis_plan["steps"][:2] if step["title"] not in next_moves)
    if chart_recommendations:
        chart = chart_recommendations[0]
        next_moves.append(f"Render a {chart['chart_type']} chart for {chart.get('y') or 'available measures'}.")

    return {
        "headline": headline,
        "confidence": confidence,
        "watchouts": watchouts,
        "next_moves": next_moves,
    }


def build_analysis_plan(
    schema: list[dict[str, Any]],
    quality: dict[str, Any],
    anomalies: list[dict[str, Any]],
    trends: list[dict[str, Any]],
    correlations: list[dict[str, Any]],
    recommendations: list[dict[str, Any]],
) -> dict[str, Any]:
    roles = schema_roles(schema)
    dataset_story = infer_dataset_story(schema, roles)

    steps: list[dict[str, str]] = []
    if quality["score"] < 80 or quality["missing_ratio"] > 0:
        steps.append(
            {
                "stage": "quality_gate",
                "title": "Run data quality gate before interpretation",
                "why": "Missing values, duplicates, anomalies, and small samples can distort downstream conclusions.",
            }
        )
    if roles["time_axis"] and roles["measures"]:
        steps.append(
            {
                "stage": "trend",
                "title": f"Trend review: {roles['measures'][0]} over {roles['time_axis'][0]}",
                "why": "A time axis and numeric measure were detected, so temporal movement is likely meaningful.",
            }
        )
    if roles["dimensions"] and roles["measures"]:
        steps.append(
            {
                "stage": "segment",
                "title": f"Segment comparison: {roles['measures'][0]} by {roles['dimensions'][0]}",
                "why": "A categorical dimension can explain variance in the selected measure.",
            }
        )
    if correlations:
        strongest = correlations[0]
        steps.append(
            {
                "stage": "relationship",
                "title": f"Relationship review: {strongest['left']} vs {strongest['right']}",
                "why": f"The strongest observed correlation is {strongest['correlation']} ({strongest['strength']}).",
            }
        )
    if anomalies:
        steps.append(
            {
                "stage": "review_queue",
                "title": "Review high-deviation records",
                "why": "Detected values far from their column distribution; these may be errors or important events.",
            }
        )
    if not steps and recommendations:
        steps.append(
            {
                "stage": recommendations[0]["type"],
                "title": recommendations[0]["title"],
                "why": recommendations[0]["reason"],
            }
        )
    if not steps:
        steps.append(
            {
                "stage": "overview",
                "title": "Start with schema and distribution overview",
                "why": "The file has limited analyzable structure, so profiling should come before interpretation.",
            }
        )

    return {
        "dataset_story": dataset_story,
        "roles": roles,
        "confidence": "high" if quality["score"] >= 80 and len(steps) >= 2 else "medium" if quality["score"] >= 60 else "low",
        "steps": steps[:5],
    }


def infer_dataset_story(schema: list[dict[str, Any]], roles: dict[str, list[str]]) -> str:
    names = " ".join(item["name"].lower() for item in schema)
    if any(token in names for token in ["sales", "revenue", "order", "customer", "销售", "收入", "订单", "客户"]):
        return "This looks like a sales or operations table."
    if any(token in names for token in ["price", "cost", "profit", "amount", "finance", "金额", "价格", "成本", "利润"]):
        return "This looks like a financial or transaction table."
    if roles["time_axis"] and roles["measures"]:
        return "This looks like a time-series measurement table."
    if roles["dimensions"] and roles["measures"]:
        return "This looks like a dimensional analysis table."
    return "This looks like a generic tabular dataset."


def build_markdown_report(profile: dict[str, Any]) -> str:
    dataset = profile["dataset"]
    quality = profile["quality"]
    session = profile.get("session", {})
    source = profile.get("source", {})
    diagnostics = profile.get("table_diagnostics", {})
    fingerprint = profile.get("dataset_fingerprint", {})
    repair_plan = profile.get("quality_repair_plan", [])
    insight_cards = profile.get("insight_cards", [])
    recommended_views = profile.get("recommended_views", [])
    local_ai = profile.get("local_ai", {})
    lines = [
        f"# TablePilot Analysis Report: {dataset['filename']}",
        "",
        "## Executive Summary",
        profile.get("executive_brief", {}).get("headline", ""),
        "",
        "## Session",
        f"- Session ID: {session.get('id', '')}",
        f"- Generated at: {session.get('generated_at', '')}",
        f"- Source parser: {source.get('parser', '')}",
        f"- Sheet: {source.get('sheet_name') or '-'}",
        f"- Encoding: {source.get('encoding') or '-'}",
        f"- Delimiter: {source.get('delimiter') or '-'}",
        "",
        "## Dataset Fingerprint",
        f"- Type: {fingerprint.get('label', 'Unknown')}",
        f"- Confidence: {fingerprint.get('confidence', '')}",
        f"- Summary: {fingerprint.get('summary', '')}",
        f"- Messy score: {diagnostics.get('messy_score', 0)}/100",
        "",
        "## Dataset Profile",
        f"- Rows: {dataset['rows']}",
        f"- Columns: {dataset['columns']}",
        f"- Numeric columns: {dataset['numeric_columns']}",
        f"- Date columns: {dataset['date_columns']}",
        f"- Category columns: {dataset['category_columns']}",
        "",
        "## Data Quality",
        f"- Score: {quality['score']}/100 ({quality['level']})",
        f"- Missing ratio: {quality['missing_ratio']}",
        f"- Duplicate rows: {quality['duplicate_rows']}",
        f"- Anomaly count: {quality['anomaly_count']}",
        f"- Dropped empty rows: {diagnostics.get('dropped_empty_rows', 0)}",
        f"- Dropped empty columns: {diagnostics.get('dropped_empty_columns', 0)}",
        "",
        "## Insight Cards",
    ]
    for card in insight_cards:
        lines.append(f"- **{card['title']}**: {card['summary']} Evidence: {card['evidence']}")
    if repair_plan:
        lines.extend(["", "## Data Quality Repair Plan"])
        for item in repair_plan:
            lines.append(f"- **{item['title']}** ({item['severity']}): {item['recommendation']} Evidence: {item['evidence']}")
    if recommended_views:
        lines.extend(["", "## Recommended Views"])
        for view in recommended_views:
            lines.append(f"- **{view['label']}**: {view['chart_type']} using x={view['x'] or '-'}, y={view['y'] or '-'} - {view['reason']}")
    lines.extend(["", "## Local AI Layer"])
    lines.append(f"- Provider: {local_ai.get('provider', 'ollama')}")
    lines.append(f"- Model: {local_ai.get('model', '')}")
    lines.append(f"- Status: {local_ai.get('status', '')}")
    lines.append(f"- Guardrail: {local_ai.get('guardrail', '')}")
    if local_ai.get("summary"):
        lines.append(f"- Summary: {local_ai['summary']}")
    plan = profile.get("analysis_plan", {})
    if plan:
        lines.extend(["", "## Analysis Plan"])
        lines.append(f"- Dataset story: {plan.get('dataset_story', '')}")
        lines.append(f"- Planner confidence: {plan.get('confidence', '')}")
        lines.extend(f"- {item['stage']}: {item['title']} - {item['why']}" for item in plan.get("steps", []))
    lines.extend(["", "## Recommended Actions"])
    lines.extend(f"- **{item['type']}**: {item['title']} - {item['reason']}" for item in profile["analysis_recommendations"])
    lines.extend(["", "## Analysis Basis"])
    lines.append("This report is generated locally from schema inference, quality scoring, trend detection, correlation review, and anomaly checks.")
    lines.append("It is an exploratory analytical report, not a business recommendation.")
    return "\n".join(lines) + "\n"


def build_html_report(profile: dict[str, Any]) -> str:
    markdown = build_markdown_report(profile)
    body = "\n".join(markdown_to_basic_html(line) for line in markdown.splitlines())
    return (
        "<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
        "<title>TablePilot Analysis Report</title>"
        "<style>body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;max-width:920px;margin:48px auto;padding:0 24px;color:#1d1d1f;line-height:1.55}"
        "h1{font-size:34px}h2{margin-top:30px;border-top:1px solid #e5e5ea;padding-top:18px}"
        "li{margin:6px 0}code{background:#f5f5f7;padding:2px 5px;border-radius:6px}</style>"
        "</head><body>"
        + body
        + "</body></html>"
    )


def markdown_to_basic_html(line: str) -> str:
    text = line.strip()
    if not text:
        return ""
    if text.startswith("# "):
        return f"<h1>{text[2:]}</h1>"
    if text.startswith("## "):
        return f"<h2>{text[3:]}</h2>"
    if text.startswith("- "):
        return f"<li>{text[2:]}</li>"
    return f"<p>{text}</p>"


def _safe_float(value: Any) -> float | None:
    try:
        result = float(value)
    except (TypeError, ValueError):
        return None
    if math.isnan(result) or math.isinf(result):
        return None
    return round(result, 4)
