from pathlib import Path

from fastapi.testclient import TestClient

from app.analysis import load_table_with_metadata, list_datasets, profile_dataset, profile_table
from app.main import app


client = TestClient(app)
ROOT = Path(__file__).resolve().parents[2]
FIXTURES = ROOT / "analysis_service" / "tests" / "fixtures"
DEMO = ROOT / "demo"


def test_lists_demo_datasets():
    datasets = list_datasets(DEMO)
    names = {item["filename"] for item in datasets}

    assert "latticeiq_demo_sales.xlsx" in names


def test_profiles_demo_excel_dataset():
    profile = profile_dataset("latticeiq_demo_sales.xlsx", DEMO)

    assert profile["dataset"]["rows"] == 12
    assert profile["dataset"]["columns"] == 9
    assert profile["dataset"]["numeric_columns"] >= 4
    assert profile["quality"]["score"] > 0
    assert profile["tool_trace"]
    assert profile["executive_brief"]["headline"]
    assert profile["executive_brief"]["next_moves"]
    assert profile["analysis_plan"]["dataset_story"]
    assert profile["analysis_plan"]["steps"]
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


def test_markdown_report_endpoint():
    response = client.post("/api/report/markdown", json={"filename": "latticeiq_demo_sales.xlsx"})

    assert response.status_code == 200
    assert "# LatticeIQ Analysis Report" in response.text
    assert "## Executive Brief" in response.text
    assert "## Analysis Plan" in response.text


def test_dataset_alias_for_report_endpoint():
    response = client.post("/api/report/markdown", json={"dataset": "latticeiq_demo_sales.xlsx"})

    assert response.status_code == 200
    assert "# LatticeIQ Analysis Report" in response.text


def test_health_endpoint():
    response = client.get("/health")

    assert response.status_code == 200
    assert response.json() == {"status": "ok"}


def test_analyze_endpoint():
    response = client.post("/api/analyze", json={"filename": "latticeiq_demo_sales.xlsx"})

    assert response.status_code == 200
    body = response.json()
    assert body["dataset"]["filename"] == "latticeiq_demo_sales.xlsx"
    assert body["dataset"]["rows"] == 12
    assert body["quality"]["level"] in {"high", "medium", "low"}
    assert body["analysis_recommendations"]
    assert body["chart_recommendations"]


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


def test_agent_query_endpoint():
    response = client.post(
        "/api/agent/query",
        json={"filename": "latticeiq_demo_sales.xlsx", "question": "这份数据有没有异常？"},
    )

    assert response.status_code == 200
    body = response.json()
    assert body["intent"] == "anomaly_review"
    assert "detect_anomalies" in body["tools_used"]
    assert body["llm_status"] == "disabled"


def test_dataset_alias_for_agent_query_endpoint():
    response = client.post(
        "/api/agent/query",
        json={"dataset": "latticeiq_demo_sales.xlsx", "question": "哪些字段适合做趋势分析？"},
    )

    assert response.status_code == 200
    body = response.json()
    assert body["intent"] == "trend_review"


def test_analyze_rejects_unknown_file():
    response = client.post("/api/analyze", json={"filename": "missing.xlsx"})

    assert response.status_code == 404
