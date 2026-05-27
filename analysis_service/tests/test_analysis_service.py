from pathlib import Path

from fastapi.testclient import TestClient

from app.analysis import load_table_with_metadata, profile_dataset, profile_table
from app.main import app


client = TestClient(app)
ROOT = Path(__file__).resolve().parents[2]


def test_lists_sample_datasets():
    from app.analysis import list_datasets

    datasets = list_datasets()
    names = {item["filename"] for item in datasets}

    assert "销售数据.txt" in names
    assert "销售数据.xlsx" in names


def test_profiles_text_dataset():
    profile = profile_dataset("销售数据.txt")

    assert profile["dataset"]["rows"] == 6
    assert profile["dataset"]["columns"] == 6
    assert profile["dataset"]["numeric_columns"] == 6
    assert profile["quality"]["score"] > 0
    assert len(profile["columns"]) == 6
    assert len(profile["schema"]) == 6
    assert profile["tool_trace"]
    assert "trends" in profile
    assert "correlations" in profile
    assert profile["preview"]["returned_rows"] == 6
    assert len(profile["preview"]["rows"][0]) == 6
    assert profile["insights"]


def test_profiles_excel_dataset():
    profile = profile_dataset("销售数据.xlsx")

    assert profile["dataset"]["rows"] >= 5
    assert profile["dataset"]["columns"] == 6
    assert profile["dataset"]["numeric_columns"] == 6


def test_detects_tab_delimited_txt():
    loaded = load_table_with_metadata(ROOT / "samples" / "txt_tab_sample.txt")
    profile = profile_table("txt_tab_sample.txt", ".txt", loaded["frame"], loaded["source"])

    assert loaded["source"]["delimiter"] == "tab"
    assert loaded["source"]["has_header"] is True
    assert profile["dataset"]["rows"] == 4
    assert profile["dataset"]["numeric_columns"] == 2
    assert any(item["semantic_type"] == "date" for item in profile["schema"])


def test_detects_whitespace_txt():
    loaded = load_table_with_metadata(ROOT / "samples" / "txt_space_sample.txt")
    profile = profile_table("txt_space_sample.txt", ".txt", loaded["frame"], loaded["source"])

    assert loaded["source"]["delimiter"] == "whitespace"
    assert profile["dataset"]["rows"] == 4
    assert profile["dataset"]["numeric_columns"] == 2


def test_schema_and_recommendations_for_mixed_csv():
    loaded = load_table_with_metadata(ROOT / "samples" / "mixed_schema_sample.csv")
    profile = profile_table("mixed_schema_sample.csv", ".csv", loaded["frame"], loaded["source"])
    schema = {item["name"]: item["semantic_type"] for item in profile["schema"]}

    assert schema["order_date"] == "date"
    assert schema["revenue"] == "numeric"
    assert schema["region"] == "category"
    assert profile["analysis_recommendations"]
    assert profile["chart_recommendations"][0]["chart_type"] in {"line", "bar"}


def test_missing_values_affect_quality_score():
    loaded = load_table_with_metadata(ROOT / "samples" / "missing_values_sample.csv")
    profile = profile_table("missing_values_sample.csv", ".csv", loaded["frame"], loaded["source"])

    assert profile["dataset"]["missing_cells"] > 0
    assert profile["quality"]["score"] < 100


def test_markdown_report_endpoint():
    response = client.post("/api/report/markdown", json={"filename": "销售数据.txt"})

    assert response.status_code == 200
    assert "# InsightQt Analysis Report" in response.text


def test_dataset_alias_for_report_endpoint():
    response = client.post("/api/report/markdown", json={"dataset": "销售数据.txt"})

    assert response.status_code == 200
    assert "# InsightQt Analysis Report" in response.text


def test_health_endpoint():
    response = client.get("/health")

    assert response.status_code == 200
    assert response.json() == {"status": "ok"}


def test_analyze_endpoint():
    response = client.post("/api/analyze", json={"filename": "销售数据.txt"})

    assert response.status_code == 200
    body = response.json()
    assert body["dataset"]["filename"] == "销售数据.txt"
    assert body["dataset"]["rows"] == 6
    assert body["quality"]["level"] in {"high", "medium", "low"}
    assert body["analysis_recommendations"]
    assert body["chart_recommendations"]


def test_upload_analysis_endpoint():
    with open(ROOT / "Statistical_Analysis" / "销售数据.txt", "rb") as sample:
        response = client.post(
            "/api/analyze-upload",
            files={"file": ("sales.txt", sample, "text/plain")},
        )

    assert response.status_code == 200
    body = response.json()
    assert body["dataset"]["filename"] == "sales.txt"
    assert body["dataset"]["rows"] == 6
    assert body["preview"]["rows"][0][0] == 450


def test_agent_query_endpoint():
    response = client.post(
        "/api/agent/query",
        json={"filename": "销售数据.txt", "question": "这份数据有没有异常？"},
    )

    assert response.status_code == 200
    body = response.json()
    assert body["intent"] == "anomaly_review"
    assert "detect_anomalies" in body["tools_used"]
    assert body["llm_status"] == "disabled"


def test_dataset_alias_for_agent_query_endpoint():
    response = client.post(
        "/api/agent/query",
        json={"dataset": "销售数据.txt", "question": "哪些字段适合做趋势分析？"},
    )

    assert response.status_code == 200
    body = response.json()
    assert body["intent"] == "trend_review"


def test_analyze_rejects_unknown_file():
    response = client.post("/api/analyze", json={"filename": "missing.xlsx"})

    assert response.status_code == 404
