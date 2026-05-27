from __future__ import annotations

from dataclasses import dataclass
from typing import Any

from .analysis import profile_dataset


@dataclass(frozen=True)
class AgentPlan:
    intent: str
    tools: list[str]


def plan_question(question: str) -> AgentPlan:
    text = question.lower()
    if any(token in text for token in ["异常", "outlier", "anomaly", "风险"]):
        return AgentPlan("anomaly_review", ["load_dataset", "profile_dataset", "detect_anomalies", "compose_answer"])
    if any(token in text for token in ["质量", "缺失", "quality", "missing"]):
        return AgentPlan("data_quality", ["load_dataset", "profile_dataset", "score_data_quality", "compose_answer"])
    if any(token in text for token in ["趋势", "trend", "变化"]):
        return AgentPlan("trend_review", ["load_dataset", "profile_dataset", "detect_trends", "compose_answer"])
    if any(token in text for token in ["相关", "correlation", "relationship"]):
        return AgentPlan("correlation_review", ["load_dataset", "profile_dataset", "calculate_correlations", "compose_answer"])
    return AgentPlan("dataset_overview", ["load_dataset", "profile_dataset", "compose_answer"])


def answer_question(filename: str, question: str) -> dict[str, Any]:
    plan = plan_question(question)
    profile = profile_dataset(filename)
    return {
        "question": question,
        "intent": plan.intent,
        "tools_used": plan.tools,
        "tool_trace": [f"-> {tool}" for tool in plan.tools],
        "answer": compose_answer(plan.intent, profile),
        "evidence": {
            "dataset": profile["dataset"],
            "quality": profile["quality"],
            "top_anomalies": profile["anomalies"][:5],
            "top_correlations": profile["correlations"][:5],
            "top_trends": profile["trends"][:5],
        },
        "llm_status": "disabled",
        "note": "This deterministic agent layer is evidence-grounded and ready for future Ollama/OpenAI enhancement.",
    }


def compose_answer(intent: str, profile: dict[str, Any]) -> str:
    dataset = profile["dataset"]
    quality = profile["quality"]
    lines = [
        f"Dataset {dataset['filename']} contains {dataset['rows']} rows and {dataset['columns']} columns.",
        f"Data quality is {quality['level']} ({quality['score']}/100).",
    ]

    if intent == "anomaly_review":
        if profile["anomalies"]:
            lines.append(f"Found {len(profile['anomalies'])} high z-score cells. Review the largest deviations first.")
        else:
            lines.append("No high z-score anomalies were detected with the default threshold.")
    elif intent == "data_quality":
        lines.append(
            f"Missing ratio is {quality['missing_ratio']}; numeric ratio is {quality['numeric_ratio']}."
        )
    elif intent == "trend_review":
        if profile["trends"]:
            trend = profile["trends"][0]
            lines.append(f"The strongest simple trend is column {trend['column']} moving {trend['direction']}.")
    elif intent == "correlation_review":
        if profile["correlations"]:
            corr = profile["correlations"][0]
            lines.append(
                f"The strongest correlation is between columns {corr['left']} and {corr['right']} "
                f"({corr['correlation']}, {corr['strength']})."
            )
    else:
        lines.extend(profile["insights"])

    lines.append("This is not a business recommendation; it is a local analytical summary for review.")
    return "\n".join(lines)
