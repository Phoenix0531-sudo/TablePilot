from pathlib import Path

import pandas as pd
from fastapi.testclient import TestClient

from app.analysis import build_cleaned_table, load_table_with_metadata, list_datasets, profile_dataset, profile_table, validate_local_ai_summary
from app.main import app


client = TestClient(app)
ROOT = Path(__file__).resolve().parents[2]
FIXTURES = ROOT / "analysis_service" / "tests" / "fixtures"
DEMO = ROOT / "demo"


def test_lists_demo_datasets():
    datasets = list_datasets(DEMO)
    names = {item["filename"] for item in datasets}

    assert "tablepilot_demo_sales.xlsx" in names
    assert "multi_sheet_operations.xlsx" in names
    assert "quality_issues_demo.csv" in names
    assert "time_series_demo.txt" in names


def test_profiles_demo_excel_dataset():
    profile = profile_dataset("tablepilot_demo_sales.xlsx", DEMO)

    assert profile["dataset"]["rows"] == 12
    assert profile["dataset"]["columns"] == 9
    assert profile["dataset"]["numeric_columns"] >= 4
    assert profile["quality"]["score"] > 0
    assert profile["tool_trace"]
    assert profile["executive_brief"]["headline"]
    assert profile["executive_brief"]["next_moves"]
    assert profile["analysis_plan"]["dataset_story"]
    assert profile["analysis_plan"]["steps"]
    assert profile["dataset_fingerprint"]["type"] == "sales_operations"
    assert profile["dataset_fingerprint"]["roles"]["measures"]
    assert profile["insight_cards"]
    assert profile["decision_brief"]["findings"]
    assert profile["decision_brief"]["primary_question"]
    assert profile["recommended_views"]
    assert any(view["id"] == "trend" for view in profile["recommended_views"])
    assert profile["quality_repair_plan"]
    assert profile["preview"]["returned_rows"] == 12
    assert profile["insights"]


def test_detects_tab_delimited_txt():
    loaded = load_table_with_metadata(FIXTURES / "txt_tab_sample.txt")
    profile = profile_table("txt_tab_sample.txt", ".txt", loaded["frame"], loaded["source"])

    assert loaded["source"]["delimiter"] == "tab"
    assert loaded["source"]["has_header"] is True
    assert profile["dataset"]["rows"] == 4
    assert profile["dataset"]["numeric_columns"] == 2
    assert any(item["semantic_type"] == "date" for item in profile["schema"])


def test_detects_whitespace_txt():
    loaded = load_table_with_metadata(FIXTURES / "txt_space_sample.txt")
    profile = profile_table("txt_space_sample.txt", ".txt", loaded["frame"], loaded["source"])

    assert loaded["source"]["delimiter"] == "whitespace"
    assert profile["dataset"]["rows"] == 4
    assert profile["dataset"]["numeric_columns"] == 2


def test_schema_and_recommendations_for_mixed_csv():
    loaded = load_table_with_metadata(FIXTURES / "mixed_schema_sample.csv")
    profile = profile_table("mixed_schema_sample.csv", ".csv", loaded["frame"], loaded["source"])
    schema = {item["name"]: item["semantic_type"] for item in profile["schema"]}

    assert schema["order_date"] == "date"
    assert schema["revenue"] == "numeric"
    assert schema["region"] == "category"
    assert profile["analysis_recommendations"]
    assert profile["chart_recommendations"][0]["chart_type"] in {"line", "bar"}


def test_missing_values_affect_quality_score():
    loaded = load_table_with_metadata(FIXTURES / "missing_values_sample.csv")
    profile = profile_table("missing_values_sample.csv", ".csv", loaded["frame"], loaded["source"])

    assert profile["dataset"]["missing_cells"] > 0
    assert profile["quality"]["score"] < 100


def test_demo_quality_issues_produce_repair_plan():
    profile = profile_dataset("quality_issues_demo.csv", DEMO)

    repair_ids = {item["id"] for item in profile["quality_repair_plan"]}
    assert "missing_values" in repair_ids
    assert "duplicate_rows" in repair_ids
    assert profile["quality"]["score"] < 100


def test_demo_time_series_txt_gets_trend_view():
    profile = profile_dataset("time_series_demo.txt", DEMO)

    assert profile["dataset"]["date_columns"] == 1
    assert any(view["id"] == "trend" for view in profile["recommended_views"])
    assert profile["dataset_fingerprint"]["type"] == "time_series"


def test_messy_table_autopilot_detects_late_header(tmp_path):
    path = tmp_path / "messy.csv"
    path.write_text(
        "Monthly sales export,,,,\n"
        ",,,,\n"
        "date,region,revenue,orders,margin\n"
        "2026-01-01,East,1000,10,0.2\n"
        "2026-01-02,West,,12,0.3\n"
        "合计,,1000,22,\n",
        encoding="utf-8",
    )

    loaded = load_table_with_metadata(path)
    profile = profile_table("messy.csv", ".csv", loaded["frame"], loaded["source"])

    assert loaded["source"]["has_header"] is True
    assert loaded["source"]["header_row_index"] == 1
    assert profile["dataset"]["rows"] == 2
    assert profile["table_diagnostics"]["messy_score"] > 0
    assert any(item["id"] == "missing_values" for item in profile["quality_repair_plan"])


def test_excel_sheet_selection(tmp_path):
    path = tmp_path / "multi_sheet.xlsx"
    with pd.ExcelWriter(path) as writer:
        pd.DataFrame({"date": ["2026-01-01"], "revenue": [100]}).to_excel(writer, sheet_name="Summary", index=False)
        pd.DataFrame({"region": ["East", "West"], "orders": [10, 12]}).to_excel(writer, sheet_name="Detail", index=False)

    loaded = load_table_with_metadata(path, sheet_name="Detail")
    profile = profile_table("multi_sheet.xlsx", ".xlsx", loaded["frame"], loaded["source"])

    assert loaded["source"]["sheet_name"] == "Detail"
    assert loaded["source"]["sheets"] == ["Summary", "Detail"]
    assert profile["preview"]["columns"] == ["region", "orders"]


def test_demo_multi_sheet_workbook_can_select_detail_sheet():
    profile = profile_dataset("multi_sheet_operations.xlsx", DEMO, sheet_name="Detail")

    assert profile["source"]["sheet_name"] == "Detail"
    assert profile["source"]["sheets"] == ["Summary", "Detail"]
    assert "customer_segment" in profile["preview"]["columns"]


def test_local_ai_is_disabled_by_default(monkeypatch):
    monkeypatch.delenv("TABLEPILOT_ENABLE_LOCAL_AI", raising=False)
    monkeypatch.delenv("TABLEPILOT_ENABLE_OLLAMA", raising=False)
    monkeypatch.delenv("LOCAL_LLM_BASE_URL", raising=False)
    profile = profile_dataset("tablepilot_demo_sales.xlsx", DEMO)

    assert profile["local_ai"]["provider"] == "ollama"
    assert profile["local_ai"]["status"] == "disabled"


def test_local_ai_prefers_openai_compatible_provider_when_base_url_is_set(monkeypatch):
    monkeypatch.delenv("TABLEPILOT_ENABLE_LOCAL_AI", raising=False)
    monkeypatch.delenv("TABLEPILOT_ENABLE_OLLAMA", raising=False)
    monkeypatch.setenv("LOCAL_LLM_BASE_URL", "http://127.0.0.1:39281/v1")
    monkeypatch.setenv("LOCAL_LLM_MODEL", "qwen3-4b")
    profile = profile_dataset("tablepilot_demo_sales.xlsx", DEMO)

    assert profile["local_ai"]["provider"] == "openai-compatible"
    assert profile["local_ai"]["model"] == "qwen3-4b"
    assert profile["local_ai"]["status"] == "disabled"


def test_local_ai_openai_compatible_unavailable_falls_back(monkeypatch):
    monkeypatch.setenv("TABLEPILOT_ENABLE_LOCAL_AI", "1")
    monkeypatch.setenv("TABLEPILOT_LOCAL_AI_PROVIDER", "openai-compatible")
    monkeypatch.setenv("LOCAL_LLM_BASE_URL", "http://127.0.0.1:1/v1")
    monkeypatch.setenv("LOCAL_LLM_MODEL", "qwen3-4b")
    profile = profile_dataset("tablepilot_demo_sales.xlsx", DEMO)

    assert profile["local_ai"]["provider"] == "openai-compatible"
    assert profile["local_ai"]["status"] == "unavailable"
    assert profile["executive_brief"]["headline"]


def test_local_ai_can_be_requested_per_profile(monkeypatch):
    monkeypatch.delenv("TABLEPILOT_ENABLE_LOCAL_AI", raising=False)
    monkeypatch.setenv("TABLEPILOT_LOCAL_AI_PROVIDER", "openai-compatible")
    monkeypatch.setenv("LOCAL_LLM_BASE_URL", "http://127.0.0.1:1/v1")
    monkeypatch.setenv("LOCAL_LLM_MODEL", "qwen3-4b")
    profile = profile_dataset("tablepilot_demo_sales.xlsx", DEMO, local_ai_enabled=True)

    assert profile["local_ai"]["model"] == "qwen3-4b"
    assert profile["local_ai"]["status"] == "unavailable"


def test_local_ai_guardrail_rejects_unknown_metric_reference():
    profile = profile_dataset("time_series_demo.txt", DEMO)
    result = validate_local_ai_summary("metric_d is volatile but metric_a is stable.", profile)

    assert result["ok"] is False
    assert "metric_d" in result["reason"]


def test_markdown_report_endpoint():
    response = client.post("/api/report/markdown", json={"filename": "tablepilot_demo_sales.xlsx"})

    assert response.status_code == 200
    assert "# TablePilot Analysis Report" in response.text
    assert "## Executive Summary" in response.text
    assert "## Dataset Fingerprint" in response.text
    assert "## Insight Cards" in response.text
    assert "## Data Quality Repair Plan" in response.text
    assert "## Recommended Views" in response.text
    assert "## Analysis Plan" in response.text
    assert "## Tool Trace" not in response.text


def test_html_report_endpoint():
    response = client.post("/api/report/html", json={"filename": "tablepilot_demo_sales.xlsx"})

    assert response.status_code == 200
    assert "<!doctype html>" in response.text
    assert "TablePilot Analysis Report" in response.text


def test_session_export_endpoint():
    response = client.post("/api/session/export", json={"filename": "tablepilot_demo_sales.xlsx"})

    assert response.status_code == 200
    body = response.json()
    assert body["session"]["id"]
    assert body["profile"]["dataset"]["filename"] == "tablepilot_demo_sales.xlsx"


def test_dataset_alias_for_report_endpoint():
    response = client.post("/api/report/markdown", json={"dataset": "tablepilot_demo_sales.xlsx"})

    assert response.status_code == 200
    assert "# TablePilot Analysis Report" in response.text


def test_health_endpoint():
    response = client.get("/health")

    assert response.status_code == 200
    assert response.json() == {"status": "ok"}


def test_analyze_endpoint():
    response = client.post("/api/analyze", json={"filename": "tablepilot_demo_sales.xlsx"})

    assert response.status_code == 200
    body = response.json()
    assert body["dataset"]["filename"] == "tablepilot_demo_sales.xlsx"
    assert body["dataset"]["rows"] == 12
    assert body["quality"]["level"] in {"high", "medium", "low"}
    assert body["analysis_recommendations"]
    assert body["chart_recommendations"]
    assert body["dataset_fingerprint"]["label"]
    assert body["insight_cards"]
    assert body["recommended_views"]
    assert body["quality_repair_plan"]


def test_upload_analysis_endpoint():
    with open(FIXTURES / "sales_sample.csv", "rb") as sample:
        response = client.post(
            "/api/analyze-upload",
            files={"file": ("sales.csv", sample, "text/csv")},
        )

    assert response.status_code == 200
    body = response.json()
    assert body["dataset"]["filename"] == "sales.csv"
    assert body["dataset"]["rows"] == 8
    assert body["preview"]["rows"][0][0].startswith("2026-01-01")


def test_cleaned_table_marks_anomalies_and_removes_duplicates():
    df = pd.DataFrame(
        {
            "region": ["East", "East", "West", "West", None],
            "revenue": [100, 100, None, 9999, 120],
            "empty": [None, None, None, None, None],
        }
    )
    cleaned, summary = build_cleaned_table(df, "messy.csv")

    assert "empty" not in cleaned.columns
    assert summary["removed_duplicate_rows"] >= 1
    assert summary["filled_missing_cells"] >= 1
    assert summary["cleaned_rows"] <= summary["original_rows"]


def test_clean_upload_endpoint_returns_csv():
    with open(FIXTURES / "missing_values_sample.csv", "rb") as sample:
        response = client.post(
            "/api/clean-upload?format=csv",
            files={"file": ("missing.csv", sample, "text/csv")},
        )

    assert response.status_code == 200
    assert response.headers["content-type"].startswith("text/csv")
    assert b"sales" in response.content


def test_clean_upload_endpoint_returns_xlsx():
    with open(DEMO / "quality_issues_demo.csv", "rb") as sample:
        response = client.post(
            "/api/clean-upload?format=xlsx",
            files={"file": ("quality.csv", sample, "text/csv")},
        )

    assert response.status_code == 200
    assert "spreadsheetml.sheet" in response.headers["content-type"]
    assert response.content.startswith(b"PK")


def test_clean_preview_upload_endpoint_returns_before_after():
    with open(DEMO / "quality_issues_demo.csv", "rb") as sample:
        response = client.post(
            "/api/clean-preview-upload",
            files={"file": ("quality.csv", sample, "text/csv")},
        )

    assert response.status_code == 200
    body = response.json()
    assert body["summary"]["cleaned_rows"] <= body["summary"]["original_rows"]
    assert body["before"]["rows"]
    assert body["after"]["rows"]


def test_agent_query_endpoint():
    response = client.post(
        "/api/agent/query",
        json={"filename": "tablepilot_demo_sales.xlsx", "question": "这份数据有没有异常？"},
    )

    assert response.status_code == 200
    body = response.json()
    assert body["intent"] == "anomaly_review"
    assert "detect_anomalies" in body["tools_used"]
    assert body["llm_status"] == "disabled"


def test_dataset_alias_for_agent_query_endpoint():
    response = client.post(
        "/api/agent/query",
        json={"dataset": "tablepilot_demo_sales.xlsx", "question": "哪些字段适合做趋势分析？"},
    )

    assert response.status_code == 200
    body = response.json()
    assert body["intent"] == "trend_review"


def test_analyze_rejects_unknown_file():
    response = client.post("/api/analyze", json={"filename": "missing.xlsx"})

    assert response.status_code == 404
