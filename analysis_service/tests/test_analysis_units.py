"""Unit tests for the pure-logic functions in analysis.py.

The existing test_analysis_service.py tests are integration-style — they drive
the FastAPI endpoints or the full ``profile_dataset`` pipeline end-to-end and
assert top-level keys. That covers "does it run" but not "is each calculation
correct". These tests target the leaf pure functions directly:

  - looks_numeric / numeric_ratio / date_ratio / parse_dates_quietly
  - detect_trends
  - score_data_quality
  - find_duplicate_columns / count_total_like_rows / calculate_messy_score
  - schema_roles / build_dataset_fingerprint
  - preferred_metric / top_metrics / metric_priority
  - correlation_strength

A silent pandas upgrade (e.g. to_numeric coercion semantics changing) would
pass the integration tests (they only check ``status_code == 200`` and a few
keys) but break these unit assertions. Keeping them as pure-function tests
means failures point straight at the broken function, not a 3000-line pipeline.
"""

from __future__ import annotations

import warnings

import pandas as pd
import pytest

from app.analysis import (
    build_dataset_fingerprint,
    calculate_messy_score,
    correlation_strength,
    count_total_like_rows,
    date_ratio,
    detect_trends,
    find_duplicate_columns,
    looks_numeric,
    metric_priority,
    numeric_ratio,
    parse_dates_quietly,
    preferred_metric,
    schema_roles,
    score_data_quality,
    top_metrics,
)

# ---------------------------------------------------------------------------
# looks_numeric — single-value numeric coercion predicate
# ---------------------------------------------------------------------------


@pytest.mark.parametrize(
    ("value", "expected"),
    [
        (42, True),
        (3.14, True),
        ("100", True),
        ("1,000", True),        # comma thousands separator stripped
        ("1,5", True),          # parses as 1.5 after comma strip
        ("abc", False),
        ("", False),
        ("NaN", True),          # float("NaN") succeeds — matches impl behavior
        (None, False),          # pd.isna(None) -> False predicate
        (float("nan"), False),
        (pd.NA, False),
    ],
)
def test_looks_numeric(value, expected):
    assert looks_numeric(value) is expected


def test_looks_numeric_none_returns_false():
    # pd.isna(None) is True, so the function returns False — nail this so a
    # future "is None" check doesn't silently flip behavior.
    assert looks_numeric(None) is False


# ---------------------------------------------------------------------------
# numeric_ratio / date_ratio — whole-frame coercion ratios
# ---------------------------------------------------------------------------


def test_numeric_ratio_empty_frame():
    assert numeric_ratio(pd.DataFrame()) == 0.0


def test_numeric_ratio_all_numeric():
    df = pd.DataFrame({"a": [1, 2, 3], "b": [4.0, 5.0, 6.0]})
    # 6/6 cells coerce -> 1.0
    assert numeric_ratio(df) == pytest.approx(1.0)


def test_numeric_ratio_mixed():
    df = pd.DataFrame({"a": [1, 2, 3], "b": ["x", "y", "z"]})
    # 3 numeric of 6 total cells
    assert numeric_ratio(df) == pytest.approx(0.5)


def test_date_ratio_empty():
    assert date_ratio(pd.DataFrame()) == 0.0


def test_date_ratio_pure_dates():
    df = pd.DataFrame({"d": ["2024-01-01", "2024-02-01", "2024-03-01"]})
    assert date_ratio(df) == pytest.approx(1.0)


def test_date_ratio_mixed():
    # Use a non-numeric, non-date string so only the date column cells parse.
    # (Integers like [1,2,3] DO parse as dates in pd.to_datetime, which would
    # inflate the ratio.)
    df = pd.DataFrame({"d": ["2024-01-01", "not-a-date", "x"], "n": ["abc", "def", "ghi"]})
    # Only 1 of 6 cells is a parseable date
    assert date_ratio(df) == pytest.approx(1 / 6)


def test_parse_dates_quietly_suppresses_warning():
    # The whole point of parse_dates_quietly is to swallow UserWarnings from
    # pd.to_datetime on messy input. If the filter breaks, this test fails
    # because a warning escapes the context manager.
    s = pd.Series(["2024-01-01", "garbage", "2024-03-01"])
    with warnings.catch_warnings():
        warnings.simplefilter("error")
        result = parse_dates_quietly(s)
    assert pd.isna(result.iloc[1])
    assert not pd.isna(result.iloc[0])


# ---------------------------------------------------------------------------
# detect_trends — direction + slope from a numeric frame
# ---------------------------------------------------------------------------


def test_detect_trends_up():
    df = pd.DataFrame({"rev": [10.0, 20.0, 30.0, 40.0]})
    trends = detect_trends(df)
    assert len(trends) == 1
    t = trends[0]
    assert t["column"] == "rev"
    assert t["direction"] == "up"
    assert t["slope"] == pytest.approx(10.0)
    assert t["first"] == pytest.approx(10.0)
    assert t["last"] == pytest.approx(40.0)


def test_detect_trends_down():
    df = pd.DataFrame({"cost": [40.0, 30.0, 20.0, 10.0]})
    t = detect_trends(df)[0]
    assert t["direction"] == "down"
    assert t["slope"] == pytest.approx(-10.0)


def test_detect_trends_flat_constant_series():
    df = pd.DataFrame({"k": [5.0, 5.0, 5.0, 5.0]})
    t = detect_trends(df)[0]
    assert t["direction"] == "flat"
    assert t["slope"] == pytest.approx(0.0)


def test_detect_trends_skips_short_series():
    # <3 non-null points -> no trend reported for that column.
    # Columns must be equal length, so pad the short one with None.
    df = pd.DataFrame({"short": [1.0, 2.0, None], "long": [1.0, 2.0, 3.0]})
    trends = detect_trends(df)
    cols = {t["column"] for t in trends}
    assert cols == {"long"}


def test_detect_trends_sorted_by_abs_slope():
    df = pd.DataFrame({
        "big": [0.0, 100.0, 200.0],      # slope 100
        "small": [0.0, 1.0, 2.0],        # slope 1
    })
    trends = detect_trends(df)
    assert trends[0]["column"] == "big"
    assert trends[1]["column"] == "small"


def test_detect_trends_drops_na():
    df = pd.DataFrame({"a": [1.0, None, 3.0, 4.0]})
    # dropna -> 3 points -> trend reported
    trends = detect_trends(df)
    assert len(trends) == 1


# ---------------------------------------------------------------------------
# score_data_quality — composite 0-100 score
# ---------------------------------------------------------------------------


def _make_schema(names_types):
    """Build a minimal schema list like infer_schema would produce."""
    return [
        {
            "name": n,
            "semantic_type": t,
            "is_analyzable": t in ("numeric", "date", "category"),
            "role_hint": "measure" if t == "numeric" else "time_axis" if t == "date" else "dimension",
        }
        for n, t in names_types
    ]


def test_score_data_quality_clean_table():
    # 25 rows so sample_warning is False (the threshold is <20).
    df = pd.DataFrame({"a": range(25), "b": range(25, 50)})
    schema = _make_schema([("a", "numeric"), ("b", "numeric")])
    numeric_df = df.copy()
    q = score_data_quality(df, schema, numeric_df, missing_cells=0, anomalies=[])
    assert 80 <= q["score"] <= 100
    assert q["level"] == "high"
    assert q["missing_ratio"] == pytest.approx(0.0)
    assert q["duplicate_rows"] == 0
    assert q["anomaly_count"] == 0
    assert q["sample_warning"] is False


def test_score_data_quality_missing_cells_lower_score():
    clean = pd.DataFrame({"a": range(25), "b": range(25, 50)})
    # 25 rows, 2 with None in column a (same length as b).
    a_vals = [1, None, 3, None, 5] + list(range(6, 26))  # 5 + 20 = 25
    missing = pd.DataFrame({"a": a_vals, "b": list(range(25, 50))})
    schema = _make_schema([("a", "numeric"), ("b", "numeric")])
    q_clean = score_data_quality(clean, schema, clean, 0, [])
    q_missing = score_data_quality(missing, schema, missing, 2, [])
    assert q_missing["score"] < q_clean["score"]
    assert q_missing["missing_ratio"] > 0.0


def test_score_data_quality_duplicates_lower_score():
    # 25 rows, one duplicated row, vs a fully unique 25-row table.
    df = pd.DataFrame({"a": [1, 1] + list(range(3, 25)), "b": [6, 6] + list(range(8, 30))})
    schema = _make_schema([("a", "numeric"), ("b", "numeric")])
    q = score_data_quality(df, schema, df, 0, [])
    assert q["duplicate_rows"] == 1
    df2 = pd.DataFrame({"a": range(25), "b": range(25, 50)})
    q2 = score_data_quality(df2, schema, df2, 0, [])
    assert q["score"] < q2["score"]


def test_score_data_quality_small_sample_warning():
    df = pd.DataFrame({"a": [1, 2], "b": [3, 4]})  # < 5 rows
    schema = _make_schema([("a", "numeric"), ("b", "numeric")])
    q = score_data_quality(df, schema, df, 0, [])
    assert q["sample_warning"] is True


def test_score_data_quality_score_bounded_0_100():
    # Try to drive the score negative with every penalty active.
    df = pd.DataFrame({"a": [None, None, None, None, None, None, None, None, None, None]})
    schema = _make_schema([("a", "numeric")])
    q = score_data_quality(df, schema, df, 10, [{}, {}, {}, {}, {}])
    assert q["score"] >= 0
    assert q["score"] <= 100


def test_score_data_quality_levels():
    df = pd.DataFrame({"a": range(25)})
    schema = _make_schema([("a", "numeric")])
    q = score_data_quality(df, schema, df, 0, [])
    # clean 25-row numeric table -> high (>=80)
    assert q["level"] == "high"


# ---------------------------------------------------------------------------
# find_duplicate_columns / count_total_like_rows / calculate_messy_score
# ---------------------------------------------------------------------------


def test_find_duplicate_columns_detects_exact_copy():
    df = pd.DataFrame({"a": [1, 2, 3], "b": [1, 2, 3], "c": [9, 8, 7]})
    dups = find_duplicate_columns(df)
    assert {"left": "a", "right": "b"} in dups
    assert {"left": "a", "right": "c"} not in dups


def test_find_duplicate_columns_none_unique():
    df = pd.DataFrame({"a": [1, 2, 3], "b": [4, 5, 6], "c": [7, 8, 9]})
    assert find_duplicate_columns(df) == []


def test_find_duplicate_columns_capped_at_10():
    cols = {f"c{i}": [1, 2, 3] for i in range(15)}
    df = pd.DataFrame(cols)
    dups = find_duplicate_columns(df)
    assert len(dups) <= 10


@pytest.mark.parametrize(
    "first_value",
    ["total", "Total", "TOTAL", "合计", "总计", "小计", "subtotal", "Grand Total"],
)
def test_count_total_like_rows_matches(first_value):
    df = pd.DataFrame({"label": [first_value, "detail", "detail"]})
    assert count_total_like_rows(df) == 1


def test_count_total_like_rows_none():
    df = pd.DataFrame({"label": ["apple", "banana", "cherry"]})
    assert count_total_like_rows(df) == 0


def test_calculate_messy_score_zero_for_clean():
    source = {"has_header": True, "dropped_empty_rows": 0, "dropped_empty_columns": 0}
    score = calculate_messy_score(source, [], [], [], 0)
    assert score == 0


def test_calculate_messy_score_grows_with_disorder():
    source = {"has_header": False, "dropped_empty_rows": 4, "dropped_empty_columns": 4}
    # no header + dropped rows/cols -> big penalty
    score = calculate_messy_score(source, ["empty1"], [{"left": "a", "right": "b"}], ["hc1"], 2)
    assert score > 0
    # Each component capped, but total can still accumulate
    assert score <= 100  # 15+20+20+20+20+15+10 = 120 but caps per component; total bounded by sum


# ---------------------------------------------------------------------------
# schema_roles / build_dataset_fingerprint
# ---------------------------------------------------------------------------


def test_schema_roles_buckets_by_role_hint():
    schema = [
        {"name": "date", "role_hint": "time_axis", "semantic_type": "date"},
        {"name": "region", "role_hint": "dimension", "semantic_type": "category"},
        {"name": "revenue", "role_hint": "measure", "semantic_type": "numeric"},
        {"name": "id", "role_hint": "identifier", "semantic_type": "high_cardinality"},
        {"name": "note", "role_hint": "", "semantic_type": "text"},
    ]
    roles = schema_roles(schema)
    assert roles["time_axis"] == ["date"]
    assert roles["dimensions"] == ["region"]
    assert roles["measures"] == ["revenue"]
    assert roles["identifiers"] == ["id"]
    assert roles["text_fields"] == ["note"]
    assert roles["business_measures"] == []


def test_build_dataset_fingerprint_sales_table():
    schema = [
        {"name": "date", "role_hint": "time_axis", "semantic_type": "date"},
        {"name": "region", "role_hint": "dimension", "semantic_type": "category"},
        {"name": "sales", "role_hint": "measure", "semantic_type": "numeric"},
    ]
    quality = {"score": 85, "sample_warning": False}
    fp = build_dataset_fingerprint(schema, quality)
    assert fp["type"] == "sales_operations"
    assert "Sales" in fp["label"]
    assert fp["confidence"] in ("high", "medium", "low")
    assert fp["field_counts"]["measures"] == 1
    assert fp["field_counts"]["dimensions"] == 1
    assert fp["field_counts"]["time_axis"] == 1
    assert fp["primary_time_axis"] == "date"
    assert fp["primary_measure"] == "sales"
    assert fp["primary_dimension"] == "region"


def test_build_dataset_fingerprint_time_series_fallback():
    # No sales/finance keyword, but has time_axis + measure -> time_series
    schema = [
        {"name": "ts", "role_hint": "time_axis", "semantic_type": "date"},
        {"name": "value", "role_hint": "measure", "semantic_type": "numeric"},
    ]
    fp = build_dataset_fingerprint(schema, {"score": 50, "sample_warning": False})
    assert fp["type"] == "time_series"


def test_build_dataset_fingerprint_generic_table():
    schema = [{"name": "col1", "role_hint": "", "semantic_type": "text"}]
    fp = build_dataset_fingerprint(schema, {"score": 50, "sample_warning": True})
    assert fp["type"] == "generic_table"
    assert "Sample size" in fp["summary"]


def test_build_dataset_fingerprint_confidence_caps_at_95():
    schema = [
        {"name": "date", "role_hint": "time_axis", "semantic_type": "date"},
        {"name": "region", "role_hint": "dimension", "semantic_type": "category"},
        {"name": "sales", "role_hint": "measure", "semantic_type": "numeric"},
    ]
    fp = build_dataset_fingerprint(schema, {"score": 90, "sample_warning": False})
    assert fp["confidence_score"] <= 95


# ---------------------------------------------------------------------------
# preferred_metric / top_metrics / metric_priority
# ---------------------------------------------------------------------------


def test_metric_priority_revenue_higher_than_rate():
    assert metric_priority("revenue") > metric_priority("conversion_rate")


def test_metric_priority_unknown_zero():
    assert metric_priority("color") == 0


def test_preferred_metric_picks_business_measure_first():
    schema = [
        {"name": "rev", "role_hint": "business_measure", "semantic_type": "numeric"},
        {"name": "count", "role_hint": "measure", "semantic_type": "numeric"},
    ]
    assert preferred_metric(schema, ["rev", "count"]) == "rev"


def test_preferred_metric_falls_back_to_highest_priority():
    schema = [
        {"name": "count", "role_hint": "measure", "semantic_type": "numeric"},
        {"name": "revenue", "role_hint": "measure", "semantic_type": "numeric"},
    ]
    # No business_measure -> picks highest metric_priority name = revenue
    assert preferred_metric(schema, ["count", "revenue"]) == "revenue"


def test_preferred_metric_empty_returns_none():
    schema = []
    assert preferred_metric(schema, []) is None


def test_top_metrics_limits_results():
    schema = [{"name": n, "role_hint": "measure", "semantic_type": "numeric"} for n in ["a", "b", "c", "d"]]
    result = top_metrics(schema, ["a", "b", "c", "d"], limit=2)
    assert len(result) == 2


def test_top_metrics_empty_returns_empty():
    assert top_metrics([], []) == []


# ---------------------------------------------------------------------------
# correlation_strength — bucketed magnitude labels
# ---------------------------------------------------------------------------


@pytest.mark.parametrize(
    ("value", "expected"),
    [
        (0.9, "strong"),
        (-0.85, "strong"),
        (0.8, "strong"),
        (0.6, "moderate"),
        (-0.5, "moderate"),
        (0.49, "weak"),
        (0.0, "weak"),
        (-0.1, "weak"),
    ],
)
def test_correlation_strength_buckets(value, expected):
    assert correlation_strength(value) == expected


def test_correlation_strength_boundary_0_5():
    # 0.5 exactly is "moderate" (>=0.5), 0.499 is "weak"
    assert correlation_strength(0.5) == "moderate"
    assert correlation_strength(0.499) == "weak"


def test_correlation_strength_boundary_0_8():
    assert correlation_strength(0.8) == "strong"
    assert correlation_strength(0.799) == "moderate"
