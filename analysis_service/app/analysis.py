from __future__ import annotations

import csv
import io
import math
import os
import re
import warnings
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


def load_table(path: Path) -> pd.DataFrame:
    return load_table_with_metadata(path)["frame"]


def load_table_with_metadata(path: Path) -> dict[str, Any]:
    suffix = path.suffix.lower()
    if suffix in {".xlsx", ".xls"}:
        raw = pd.read_excel(path, header=None)
        frame, has_header = normalize_header(raw)
        return {
            "frame": frame,
            "source": {
                "encoding": None,
                "delimiter": None,
                "has_header": has_header,
                "parser": "pandas.read_excel",
            },
        }
    if suffix in {".csv", ".txt"}:
        text, encoding = read_text_with_fallback(path)
        delimiter = detect_delimiter(text)
        raw = read_delimited_text(text, delimiter)
        frame, has_header = normalize_header(raw)
        return {
            "frame": frame,
            "source": {
                "encoding": encoding,
                "delimiter": delimiter_label(delimiter),
                "has_header": has_header,
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
    raw = raw.dropna(axis=0, how="all").dropna(axis=1, how="all").reset_index(drop=True)
    if raw.empty:
        return raw, False

    has_header = infer_header(raw)
    if has_header:
        header = [safe_column_name(value, index) for index, value in enumerate(raw.iloc[0].tolist())]
        frame = raw.iloc[1:].reset_index(drop=True)
        frame.columns = make_unique(header)
        return frame, True

    frame = raw.copy()
    frame.columns = [f"Column {index + 1}" for index in range(len(frame.columns))]
    return frame.reset_index(drop=True), False


def infer_header(raw: pd.DataFrame) -> bool:
    if len(raw) < 2:
        return False
    first = raw.iloc[0]
    rest = raw.iloc[1:].head(10)
    first_text_ratio = sum(not looks_numeric(value) for value in first) / max(len(first), 1)
    rest_numeric_ratio = numeric_ratio(rest)
    rest_date_ratio = date_ratio(rest)
    unique_first = len({str(value).strip() for value in first if str(value).strip()}) >= max(1, len(first) // 2)
    typed_ratio = rest_numeric_ratio + rest_date_ratio
    return first_text_ratio >= 0.5 and typed_ratio >= 0.25 and unique_first


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


def profile_dataset(filename: str, data_dir: Path | None = None) -> dict[str, Any]:
    path = resolve_dataset(filename, data_dir)
    loaded = load_table_with_metadata(path)
    return profile_table(path.name, path.suffix.lower(), loaded["frame"], loaded["source"])


def profile_uploaded_table(filename: str, df: pd.DataFrame) -> dict[str, Any]:
    suffix = Path(filename).suffix.lower()
    if suffix not in SUPPORTED_EXTENSIONS:
        raise ValueError(f"Unsupported dataset type: {suffix}")
    frame, has_header = normalize_header(df)
    source = {"encoding": None, "delimiter": None, "has_header": has_header, "parser": "upload"}
    return profile_table(Path(filename).name, suffix, frame, source)


def profile_table(filename: str, extension: str, df: pd.DataFrame, source: dict[str, Any] | None = None) -> dict[str, Any]:
    df = df.reset_index(drop=True)
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
    analysis_plan = build_analysis_plan(schema, quality, anomalies, trends, correlations, analysis_recommendations)
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
        "infer_schema",
        "profile_quality",
        "detect_anomalies",
        "recommend_analysis",
        "compose_insight",
    ]

    return {
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
        "source": source or {},
        "schema": schema,
        "quality": quality,
        "columns": columns,
        "anomalies": anomalies,
        "correlations": correlations,
        "trends": trends,
        "chart_recommendations": chart_recommendations,
        "analysis_recommendations": analysis_recommendations,
        "analysis_plan": analysis_plan,
        "executive_brief": executive_brief,
        "preview": build_table_preview(df),
        "tool_trace": tool_trace,
        "insights": build_insights(df, schema, missing_cells, anomalies, quality, trends, analysis_recommendations),
    }


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
    roles = {
        "time_axis": [item["name"] for item in schema if item["role_hint"] == "time_axis"],
        "dimensions": [item["name"] for item in schema if item["role_hint"] == "dimension"],
        "measures": [item["name"] for item in schema if item["role_hint"] in {"measure", "business_measure"}],
        "identifiers": [item["name"] for item in schema if item["role_hint"] == "identifier"],
    }
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
    lines = [
        f"# TablePilot Analysis Report: {dataset['filename']}",
        "",
        "## Dataset",
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
        "",
        "## Key Findings",
    ]
    lines.extend(f"- {item}" for item in profile["insights"])
    brief = profile.get("executive_brief", {})
    if brief:
        lines.extend(["", "## Executive Brief"])
        lines.append(f"- Headline: {brief.get('headline', '')}")
        lines.append(f"- Confidence: {brief.get('confidence', '')}")
        lines.extend(f"- Watchout: {item}" for item in brief.get("watchouts", []))
        lines.extend(f"- Next move: {item}" for item in brief.get("next_moves", []))
    plan = profile.get("analysis_plan", {})
    if plan:
        lines.extend(["", "## Analysis Plan"])
        lines.append(f"- Dataset story: {plan.get('dataset_story', '')}")
        lines.append(f"- Planner confidence: {plan.get('confidence', '')}")
        lines.extend(f"- {item['stage']}: {item['title']} - {item['why']}" for item in plan.get("steps", []))
    lines.extend(["", "## Recommended Analysis"])
    lines.extend(f"- **{item['type']}**: {item['title']} - {item['reason']}" for item in profile["analysis_recommendations"])
    lines.extend(["", "## Tool Trace"])
    lines.extend(f"- {item}" for item in profile["tool_trace"])
    return "\n".join(lines) + "\n"


def _safe_float(value: Any) -> float | None:
    try:
        result = float(value)
    except (TypeError, ValueError):
        return None
    if math.isnan(result) or math.isinf(result):
        return None
    return round(result, 4)
