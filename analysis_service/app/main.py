from __future__ import annotations

from pathlib import Path
from tempfile import NamedTemporaryFile

from fastapi import FastAPI, HTTPException, UploadFile
from fastapi.responses import PlainTextResponse
from pydantic import BaseModel, Field, model_validator

from .agent import answer_question
from .analysis import build_markdown_report, list_datasets, load_table_with_metadata, profile_dataset, profile_table

app = FastAPI(
    title="InsightQt Analysis Service",
    description="Local data profiling service for the Qt statistical analysis workbench.",
    version="0.4.0",
)


class DatasetRequest(BaseModel):
    filename: str | None = Field(
        default=None,
        min_length=1,
        description="Dataset filename under the configured data directory.",
    )
    dataset: str | None = Field(default=None, min_length=1, description="Alias for filename.")

    @model_validator(mode="after")
    def require_dataset_name(self) -> "DatasetRequest":
        if self.filename is None and self.dataset is None:
            raise ValueError("filename or dataset is required")
        if self.filename is None:
            self.filename = self.dataset
        return self


class AnalyzeRequest(DatasetRequest):
    pass


class AgentRequest(DatasetRequest):
    question: str = Field(..., min_length=1)


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


@app.post("/api/analyze-upload")
async def analyze_upload(file: UploadFile) -> dict[str, object]:
    suffix = Path(file.filename or "").suffix.lower()
    try:
        with NamedTemporaryFile(delete=False, suffix=suffix) as temp_file:
            temp_file.write(await file.read())
            temp_path = Path(temp_file.name)
        loaded = load_table_with_metadata(temp_path)
        return profile_table(file.filename or temp_path.name, suffix, loaded["frame"], loaded["source"])
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc
    finally:
        if "temp_path" in locals() and temp_path.exists():
            temp_path.unlink()


@app.post("/api/agent/query")
def agent_query(request: AgentRequest) -> dict[str, object]:
    try:
        return answer_question(request.filename, request.question)
    except FileNotFoundError as exc:
        raise HTTPException(status_code=404, detail=str(exc)) from exc
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc


@app.post("/api/report/markdown", response_class=PlainTextResponse)
def markdown_report(request: AnalyzeRequest) -> str:
    try:
        return build_markdown_report(profile_dataset(request.filename))
    except FileNotFoundError as exc:
        raise HTTPException(status_code=404, detail=str(exc)) from exc
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc
