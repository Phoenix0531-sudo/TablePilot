from __future__ import annotations

import math
import os
from pathlib import Path
from typing import Any

import pandas as pd

SUPPORTED_EXTENSIONS = {".xlsx", ".xls", ".csv", ".txt"}


def default_data_dir() -> Path:
    configured = os.getenv("DATA_DIR")
    if configured:
        return Path(configured).resolve()
    return Path(__file__).resolve().parents[2] / "Statistical_Analysis"


def list_datasets(data_dir: Path | None = None) -> list[dict[str, Any]]:
    root = (data_dir or default_data_dir()).resolve()
    if not root.exists():
        return []

    datasets = []
    for path in sorted(root.iterdir(), key=lambda item: item.name.lower()):
        if path.is_file() and path.suffix.lower() in SUPPORTED_EXTENSIONS:
            datasets.append(
                {
                    "filename": path.name,
                    "extension": path.suffix.lower(),
                    "size_bytes": path.stat().st_size,
                }
            )
    return datasets


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
    suffix = path.suffix.lower()
    if suffix in {".xlsx", ".xls"}:
        return pd.read_excel(path, header=None)
    if suffix == ".csv":
        return pd.read_csv(path, header=None)
    if suffix == ".txt":
        return pd.read_csv(path, header=None, sep=None, engine="python")
    raise ValueError(f"Unsupported dataset type: {suffix}")


def profile_dataset(filename: str, data_dir: Path | None = None) -> dict[str, Any]:
    path = resolve_dataset(filename, data_dir)
    df = load_table(path)
    numeric_df = df.apply(pd.to_numeric, errors="coerce")
    usable_numeric = numeric_df.dropna(axis=1, how="all")
    missing_cells = int(df.isna().sum().sum())

    column_profiles = []
    for column in usable_numeric.columns:
        series = usable_numeric[column].dropna()
        if series.empty:
            continue
        column_profiles.append(
            {
                "column": str(column),
                "count": int(series.count()),
                "mean": _safe_float(series.mean()),
                "std": _safe_float(series.std(ddof=0)),
                "min": _safe_float(series.min()),
                "median": _safe_float(series.median()),
                "max": _safe_float(series.max()),
            }
        )

    anomalies = detect_anomalies(usable_numeric)
    return {
        "dataset": {
            "filename": path.name,
            "extension": path.suffix.lower(),
            "rows": int(df.shape[0]),
            "columns": int(df.shape[1]),
            "numeric_columns": int(len(usable_numeric.columns)),
            "missing_cells": missing_cells,
        },
        "columns": column_profiles,
        "anomalies": anomalies,
        "insights": build_insights(df, usable_numeric, missing_cells, anomalies),
    }


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


def build_insights(
    raw_df: pd.DataFrame,
    numeric_df: pd.DataFrame,
    missing_cells: int,
    anomalies: list[dict[str, Any]],
) -> list[str]:
    insights = [
        f"Loaded {raw_df.shape[0]} rows and {raw_df.shape[1]} columns.",
        f"Detected {len(numeric_df.columns)} numeric columns for statistical profiling.",
    ]
    if missing_cells:
        insights.append(f"Found {missing_cells} missing cells; downstream analysis should account for incomplete data.")
    else:
        insights.append("No missing cells were detected in the loaded table.")
    if anomalies:
        insights.append(f"Detected {len(anomalies)} high z-score cells for review.")
    else:
        insights.append("No high z-score anomalies were detected with the default threshold.")
    return insights


def _safe_float(value: Any) -> float | None:
    try:
        result = float(value)
    except (TypeError, ValueError):
        return None
    if math.isnan(result) or math.isinf(result):
        return None
    return round(result, 4)
