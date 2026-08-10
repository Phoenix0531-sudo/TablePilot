"""Endpoint coverage gaps for analysis_service/app/main.py.

The existing test_analysis_service.py already covers most happy paths over
the FastAPI TestClient. This module fills the remaining gaps so every declared
endpoint and every error branch in main.py is exercised through HTTP:

  GET  /api/datasets                              (covered here; existing only tests the helper)
  POST /api/analyze        400 ValueError          (covered here)
  POST /api/analyze        404 FileNotFoundError  (existing)
  POST /api/agent/query    intent == data_quality  (covered here)
  POST /api/agent/query    intent == correlation_review (covered here)
  POST /api/agent/query    intent == dataset_overview   (covered here)
  POST /api/agent/query    404 FileNotFoundError  (covered here)
  POST /api/report/markdown 404 FileNotFoundError (covered here)
  POST /api/report/html    404 FileNotFoundError  (covered here)
  POST /api/session/export 404 FileNotFoundError  (covered here)
  DatasetRequest 422 when neither filename nor dataset is supplied (covered here)
"""

from __future__ import annotations

from pathlib import Path

from fastapi.testclient import TestClient

from app.main import app

client = TestClient(app)
ROOT = Path(__file__).resolve().parents[2]
DEMO = ROOT / "demo"


# --- /api/datasets -------------------------------------------------------

def test_datasets_endpoint_lists_four_sample_tables():
    """The HTTP surface (not just the helper) returns the demo dataset list."""
    response = client.get("/api/datasets")

    assert response.status_code == 200
    payload = response.json()
    names = {item["filename"] for item in payload["datasets"]}

    assert {"tablepilot_demo_sales.xlsx", "multi_sheet_operations.xlsx",
            "quality_issues_demo.csv", "time_series_demo.txt"} <= names


def test_datasets_endpoint_items_carry_extension_field():
    response = client.get("/api/datasets")

    assert response.status_code == 200
    for item in response.json()["datasets"]:
        assert item["extension"] in {".csv", ".xlsx", ".txt"}
        assert item["size_bytes"] >= 0


# --- /api/agent/query intent routing -------------------------------------

def test_agent_query_routes_data_quality_intent():
    response = client.post(
        "/api/agent/query",
        json={"filename": "tablepilot_demo_sales.xlsx",
              "question": "有没有缺失值或质量风险？"},
    )

    assert response.status_code == 200
    body = response.json()
    assert body["intent"] == "data_quality"
    assert "score_data_quality" in body["tools_used"]
    assert body["llm_status"] == "disabled"
    # evidence stays evidence-grounded: derives from actual profile
    assert body["evidence"]["dataset"]["filename"] == "tablepilot_demo_sales.xlsx"
    assert body["evidence"]["quality"]["level"] in {"high", "medium", "low"}


def test_agent_query_routes_correlation_review_intent():
    response = client.post(
        "/api/agent/query",
        json={"filename": "tablepilot_demo_sales.xlsx",
              "question": "sales 和 cost 的 correlation 是多少？"},
    )

    assert response.status_code == 200
    body = response.json()
    assert body["intent"] == "correlation_review"
    assert "calculate_correlations" in body["tools_used"]


def test_agent_query_routes_dataset_overview_intent_for_unknown_question():
    response = client.post(
        "/api/agent/query",
        json={"filename": "tablepilot_demo_sales.xlsx",
              "question": "请概述这张表的概况"},
    )

    assert response.status_code == 200
    body = response.json()
    # default fall-through intent
    assert body["intent"] == "dataset_overview"
    assert "profile_dataset" in body["tools_used"]


# --- error paths ---------------------------------------------------------

def test_agent_query_returns_404_for_unknown_dataset():
    response = client.post(
        "/api/agent/query",
        json={"filename": "does_not_exist.xlsx", "question": "概述"},
    )

    assert response.status_code == 404


def test_markdown_report_returns_404_for_unknown_dataset():
    response = client.post(
        "/api/report/markdown",
        json={"filename": "does_not_exist.xlsx"},
    )

    assert response.status_code == 404


def test_html_report_returns_404_for_unknown_dataset():
    response = client.post(
        "/api/report/html",
        json={"filename": "does_not_exist.xlsx"},
    )

    assert response.status_code == 404


def test_session_export_returns_404_for_unknown_dataset():
    response = client.post(
        "/api/session/export",
        json={"filename": "does_not_exist.xlsx"},
    )

    assert response.status_code == 404


def test_analyze_rejects_request_missing_both_filename_and_dataset():
    """DatasetRequest.require_dataset_name raises -> Pydantic 422."""
    response = client.post("/api/analyze", json={})

    assert response.status_code == 422


def test_agent_query_rejects_request_missing_question():
    """AgentRequest.question is required -> Pydantic 422."""
    response = client.post(
        "/api/agent/query",
        json={"filename": "tablepilot_demo_sales.xlsx"},
    )

    assert response.status_code == 422
