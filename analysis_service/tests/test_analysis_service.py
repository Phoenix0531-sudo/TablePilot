from pathlib import Path

from fastapi.testclient import TestClient

from app.analysis import list_datasets, profile_dataset
from app.main import app


client = TestClient(app)
ROOT = Path(__file__).resolve().parents[2]


def test_lists_sample_datasets():
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
    assert "trends" in profile
    assert "correlations" in profile
    assert profile["insights"]


def test_profiles_excel_dataset():
    profile = profile_dataset("销售数据.xlsx")

    assert profile["dataset"]["rows"] >= 5
    assert profile["dataset"]["columns"] == 6
    assert profile["dataset"]["numeric_columns"] == 6


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


def test_analyze_rejects_unknown_file():
    response = client.post("/api/analyze", json={"filename": "missing.xlsx"})

    assert response.status_code == 404
