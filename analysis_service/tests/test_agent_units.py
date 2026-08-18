"""Unit tests for the pure-logic functions in agent.py.

The existing endpoint tests drive ``/api/agent/query`` over HTTP and only
assert intent routing + status 200. They do not verify the textual
``compose_answer`` output or the ``answer_question`` evidence envelope shape.
A refactor of ``compose_answer`` wording or evidence keys would pass those
integration tests but silently change the user-facing summary.

These tests target the leaf functions directly:

  - plan_question    — keyword → AgentPlan mapping for all 5 intents
  - compose_answer    — narrative assembly per intent (empty + populated)
  - answer_question   — full envelope with profile_dataset monkeypatched

``answer_question`` is the only function that touches ``profile_dataset``;
we monkeypatch it to a synthetic fixture so no real file I/O is needed.
"""

from __future__ import annotations

import pytest

import app.agent as agent_mod
from app.agent import AgentPlan, compose_answer, plan_question

# ---------------------------------------------------------------------------
# plan_question — keyword → intent routing
# ---------------------------------------------------------------------------


@pytest.mark.parametrize(
    ("question", "expected_intent"),
    [
        # anomaly_review — CN + EN keywords
        ("这张表有哪些异常值？", "anomaly_review"),
        ("show me the outlier rows", "anomaly_review"),
        ("any anomaly in this data?", "anomaly_review"),
        ("数据风险点在哪", "anomaly_review"),
        # data_quality
        ("数据质量如何？", "data_quality"),
        ("are there missing values?", "data_quality"),
        ("quality check", "data_quality"),
        ("缺失值多吗", "data_quality"),
        # trend_review
        ("有没有趋势变化？", "trend_review"),
        ("what is the trend over time?", "trend_review"),
        ("变化趋势", "trend_review"),
        # correlation_review
        ("两列相关性如何？", "correlation_review"),
        ("show the correlation matrix", "correlation_review"),
        ("relationship between x and y", "correlation_review"),
        # dataset_overview — fallback
        ("这张表讲了什么？", "dataset_overview"),
        ("summarize this table", "dataset_overview"),
        ("hello", "dataset_overview"),
    ],
)
def test_plan_question_routes_intent(question: str, expected_intent: str) -> None:
    plan = plan_question(question)
    assert plan.intent == expected_intent


def test_plan_question_returns_agentplan_dataclass() -> None:
    plan = plan_question("quality please")
    assert isinstance(plan, AgentPlan)
    assert plan.tools[0] == "load_dataset"
    assert plan.tools[-1] == "compose_answer"


@pytest.mark.parametrize(
    ("intent", "expected_tool"),
    [
        ("anomaly_review", "detect_anomalies"),
        ("data_quality", "score_data_quality"),
        ("trend_review", "detect_trends"),
        ("correlation_review", "calculate_correlations"),
    ],
)
def test_plan_question_intent_has_distinct_tool(intent: str, expected_tool: str) -> None:
    """Each non-overview intent injects a distinct analysis tool."""
    # Pick a keyword guaranteed to route to the intent.
    keyword_map = {
        "anomaly_review": "anomaly",
        "data_quality": "quality",
        "trend_review": "trend",
        "correlation_review": "correlation",
    }
    plan = plan_question(keyword_map[intent])
    assert plan.intent == intent
    assert expected_tool in plan.tools


def test_plan_question_case_insensitive() -> None:
    assert plan_question("QUALITY CHECK").intent == "data_quality"
    assert plan_question("Anomaly?").intent == "anomaly_review"


def test_plan_question_first_match_wins_when_keywords_overlap() -> None:
    """anomaly is checked before quality, so a question with both keywords
    routes to anomaly_review (deterministic ordering)."""
    plan = plan_question("anomaly and quality")
    assert plan.intent == "anomaly_review"


# ---------------------------------------------------------------------------
# compose_answer — narrative assembly per intent
# ---------------------------------------------------------------------------


def _base_profile() -> dict:
    """A minimal profile dict satisfying all keys compose_answer reads."""
    return {
        "dataset": {"filename": "demo.csv", "rows": 100, "columns": 5},
        "quality": {
            "level": "fair",
            "score": 72,
            "missing_ratio": 0.1,
            "analyzable_ratio": 0.9,
        },
        "anomalies": [],
        "correlations": [],
        "trends": [],
        "insights": ["insight one", "insight two"],
    }


def test_compose_answer_anomaly_review_with_anomalies() -> None:
    profile = _base_profile()
    profile["anomalies"] = [{"cell": "A1"}, {"cell": "B2"}, {"cell": "C3"}]
    text = compose_answer("anomaly_review", profile)
    assert "3 high z-score cells" in text
    assert "largest deviations" in text


def test_compose_answer_anomaly_review_without_anomalies() -> None:
    text = compose_answer("anomaly_review", _base_profile())
    assert "No high z-score anomalies" in text


def test_compose_answer_data_quality_includes_ratios() -> None:
    text = compose_answer("data_quality", _base_profile())
    assert "0.1" in text
    assert "0.9" in text
    assert "Missing ratio" in text


def test_compose_answer_trend_review_with_trends() -> None:
    profile = _base_profile()
    profile["trends"] = [{"column": "sales", "direction": "up"}]
    text = compose_answer("trend_review", profile)
    assert "sales" in text
    assert "up" in text


def test_compose_answer_trend_review_without_trends() -> None:
    """No trend branch: compose_answer still returns the header + footer,
    just without a trend line."""
    text = compose_answer("trend_review", _base_profile())
    assert "100 rows and 5 columns" in text
    assert "not a business recommendation" in text


def test_compose_answer_correlation_review_with_correlations() -> None:
    profile = _base_profile()
    profile["correlations"] = [
        {"left": "a", "right": "b", "correlation": 0.92, "strength": "strong"}
    ]
    text = compose_answer("correlation_review", profile)
    assert "a" in text
    assert "b" in text
    assert "0.92" in text
    assert "strong" in text


def test_compose_answer_overview_includes_insights() -> None:
    text = compose_answer("dataset_overview", _base_profile())
    assert "insight one" in text
    assert "insight two" in text


def test_compose_answer_always_has_header_and_footer() -> None:
    for intent in ("anomaly_review", "data_quality", "trend_review",
                   "correlation_review", "dataset_overview"):
        text = compose_answer(intent, _base_profile())
        assert "demo.csv" in text
        assert "72/100" in text
        assert "not a business recommendation" in text


def test_compose_answer_reflects_quality_level() -> None:
    profile = _base_profile()
    profile["quality"]["level"] = "good"
    text = compose_answer("dataset_overview", profile)
    assert "good" in text


# ---------------------------------------------------------------------------
# answer_question — full envelope with profile_dataset monkeypatched
# ---------------------------------------------------------------------------


def _synthetic_profile() -> dict:
    """A richer profile that exercises the evidence envelope keys."""
    return {
        "dataset": {"filename": "synthetic.csv", "rows": 50, "columns": 4},
        "source": {"origin": "unit_test"},
        "schema": [{"name": "col_a", "role": "metric"}],
        "quality": {"level": "good", "score": 88, "missing_ratio": 0.0,
                     "analyzable_ratio": 1.0},
        "analysis_plan": {"steps": ["profile"]},
        "analysis_recommendations": ["fix types"],
        "chart_recommendations": [{"chart": "bar"}],
        "anomalies": [{"cell": "x"}] * 7,
        "correlations": [{"left": "a", "right": "b"}] * 3,
        "trends": [{"column": "c", "direction": "down"}] * 2,
        "insights": ["synthetic insight"],
        "tool_trace": ["load_dataset", "profile_dataset", "compose_answer"],
    }


def test_answer_question_envelope_keys(monkeypatch: pytest.MonkeyPatch) -> None:
    monkeypatch.setattr(agent_mod, "profile_dataset", lambda fn: _synthetic_profile())
    result = agent_mod.answer_question("synthetic.csv", "quality summary")

    assert result["question"] == "quality summary"
    assert result["intent"] == "data_quality"
    assert result["llm_status"] == "disabled"
    # answer is a non-empty string summary
    assert isinstance(result["answer"], str) and result["answer"]
    # tool_trace rendered as "-> name" markers
    assert result["tool_trace"][0].startswith("-> ")


def test_answer_question_evidence_truncates_to_top5(monkeypatch: pytest.MonkeyPatch) -> None:
    monkeypatch.setattr(agent_mod, "profile_dataset", lambda fn: _synthetic_profile())
    result = agent_mod.answer_question("synthetic.csv", "anomaly review")

    ev = result["evidence"]
    assert len(ev["top_anomalies"]) == 5  # profile has 7, truncated to 5
    assert len(ev["top_correlations"]) == 3  # profile has 3, stays 3
    assert len(ev["top_trends"]) == 2  # profile has 2, stays 2


def test_answer_question_evidence_carries_dataset_and_quality(monkeypatch: pytest.MonkeyPatch) -> None:
    monkeypatch.setattr(agent_mod, "profile_dataset", lambda fn: _synthetic_profile())
    result = agent_mod.answer_question("synthetic.csv", "overview")

    ev = result["evidence"]
    assert ev["dataset"]["filename"] == "synthetic.csv"
    assert ev["quality"]["score"] == 88
    assert ev["source"] == {"origin": "unit_test"}
    assert ev["schema"][0]["role"] == "metric"
    assert ev["analysis_plan"] == {"steps": ["profile"]}


def test_answer_question_falls_back_to_plan_tools_when_no_trace(monkeypatch: pytest.MonkeyPatch) -> None:
    """If profile_dataset returns no tool_trace, answer_question uses the
    plan's tool list instead of crashing."""
    profile = _synthetic_profile()
    del profile["tool_trace"]
    monkeypatch.setattr(agent_mod, "profile_dataset", lambda fn: profile)
    result = agent_mod.answer_question("synthetic.csv", "quality")

    assert result["tools_used"] == ["load_dataset", "profile_dataset",
                                    "score_data_quality", "compose_answer"]


def test_answer_question_note_mentions_local_deterministic(monkeypatch: pytest.MonkeyPatch) -> None:
    monkeypatch.setattr(agent_mod, "profile_dataset", lambda fn: _synthetic_profile())
    result = agent_mod.answer_question("synthetic.csv", "overview")
    assert "deterministic" in result["note"].lower()
