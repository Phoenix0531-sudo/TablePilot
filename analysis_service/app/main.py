from __future__ import annotations

from fastapi import FastAPI, HTTPException
from pydantic import BaseModel, Field

from .analysis import list_datasets, profile_dataset

app = FastAPI(
    title="InsightQt Analysis Service",
    description="Local data profiling service for the Qt statistical analysis workbench.",
    version="0.1.0",
)


class AnalyzeRequest(BaseModel):
    filename: str = Field(..., min_length=1, description="Dataset filename under the configured data directory.")


@app.get("/health")
def health() -> dict[str, str]:
    return {"status": "ok"}


@app.get("/api/datasets")
def datasets() -> dict[str, object]:
    return {"datasets": list_datasets()}


@app.post("/api/analyze")
def analyze(request: AnalyzeRequest) -> dict[str, object]:
    try:
        return profile_dataset(request.filename)
    except FileNotFoundError as exc:
        raise HTTPException(status_code=404, detail=str(exc)) from exc
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc
