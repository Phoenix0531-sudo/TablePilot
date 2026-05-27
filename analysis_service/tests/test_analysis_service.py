from fastapi.testclient import TestClient

from app.analysis import list_datasets, profile_dataset
from app.main import app


client = TestClient(app)


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
    assert len(profile["columns"]) == 6
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


def test_analyze_rejects_unknown_file():
    response = client.post("/api/analyze", json={"filename": "missing.xlsx"})

    assert response.status_code == 404
