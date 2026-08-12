from __future__ import annotations

import io
from pathlib import Path
from tempfile import NamedTemporaryFile

import pandas as pd
from fastapi import FastAPI, HTTPException, UploadFile
from fastapi.responses import HTMLResponse, JSONResponse, PlainTextResponse, StreamingResponse
from pydantic import BaseModel, Field, model_validator

from .agent import answer_question
from .analysis import (
    build_cleaned_table,
    build_cleaning_preview,
    build_html_report,
    build_markdown_report,
    list_datasets,
    load_table_with_metadata,
    profile_dataset,
    profile_table,
)

app = FastAPI(
    title="TablePilot Analysis Service",
    description="Local data profiling and analysis planning service for the TablePilot desktop workbench.",
    version="0.5.0",
)


class DatasetRequest(BaseModel):
    filename: str | None = Field(
        default=None,
        min_length=1,
        description="Dataset filename under the configured data directory.",
    )
    dataset: str | None = Field(default=None, min_length=1, description="Alias for filename.")
    sheet: str | None = Field(default=None, min_length=1, description="Optional Excel sheet name.")
    local_ai: bool | None = Field(default=None, description="Override optional local model enhancement.")

    @model_validator(mode="after")
    def require_dataset_name(self) -> DatasetRequest:
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
        return profile_dataset(request.filename, sheet_name=request.sheet, local_ai_enabled=request.local_ai)
    except FileNotFoundError as exc:
        raise HTTPException(status_code=404, detail=str(exc)) from exc
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc


@app.post("/api/analyze-upload")
async def analyze_upload(file: UploadFile, sheet: str | None = None, local_ai: bool | None = None) -> dict[str, object]:
    suffix = Path(file.filename or "").suffix.lower()
    try:
        with NamedTemporaryFile(delete=False, suffix=suffix) as temp_file:
            temp_file.write(await file.read())
            temp_path = Path(temp_file.name)
        loaded = load_table_with_metadata(temp_path, sheet_name=sheet)
        return profile_table(file.filename or temp_path.name, suffix, loaded["frame"], loaded["source"], local_ai_enabled=local_ai)
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
        return build_markdown_report(profile_dataset(request.filename, sheet_name=request.sheet, local_ai_enabled=request.local_ai))
    except FileNotFoundError as exc:
        raise HTTPException(status_code=404, detail=str(exc)) from exc
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc


@app.post("/api/report/html", response_class=HTMLResponse)
def html_report(request: AnalyzeRequest) -> str:
    try:
        return build_html_report(profile_dataset(request.filename, sheet_name=request.sheet, local_ai_enabled=request.local_ai))
    except FileNotFoundError as exc:
        raise HTTPException(status_code=404, detail=str(exc)) from exc
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc


@app.post("/api/session/export", response_class=JSONResponse)
def session_export(request: AnalyzeRequest) -> dict[str, object]:
    try:
        profile = profile_dataset(request.filename, sheet_name=request.sheet, local_ai_enabled=request.local_ai)
        return {"session": profile["session"], "profile": profile}
    except FileNotFoundError as exc:
        raise HTTPException(status_code=404, detail=str(exc)) from exc
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc


@app.post("/api/clean-upload")
async def clean_upload(file: UploadFile, sheet: str | None = None, format: str = "csv") -> StreamingResponse:
    suffix = Path(file.filename or "").suffix.lower()
    try:
        with NamedTemporaryFile(delete=False, suffix=suffix) as temp_file:
            temp_file.write(await file.read())
            temp_path = Path(temp_file.name)
        loaded = load_table_with_metadata(temp_path, sheet_name=sheet)
        cleaned, summary = build_cleaned_table(loaded["frame"], file.filename or temp_path.name)
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc
    finally:
        if "temp_path" in locals() and temp_path.exists():
            temp_path.unlink()

    stem = Path(file.filename or "tablepilot-cleaned").stem
    if format.lower() == "xlsx":
        output = io.BytesIO()
        with pd.ExcelWriter(output, engine="openpyxl") as writer:
            cleaned.to_excel(writer, index=False, sheet_name="cleaned")
            pd.DataFrame([summary]).to_excel(writer, index=False, sheet_name="repair_summary")
        output.seek(0)
        return StreamingResponse(
            output,
            media_type="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
            headers={"Content-Disposition": f'attachment; filename="{stem}-cleaned.xlsx"'},
        )

    output = io.StringIO()
    cleaned.to_csv(output, index=False)
    payload = io.BytesIO(output.getvalue().encode("utf-8-sig"))
    return StreamingResponse(
        payload,
        media_type="text/csv; charset=utf-8",
        headers={"Content-Disposition": f'attachment; filename="{stem}-cleaned.csv"'},
    )


@app.post("/api/clean-preview-upload")
async def clean_preview_upload(file: UploadFile, sheet: str | None = None) -> dict[str, object]:
    suffix = Path(file.filename or "").suffix.lower()
    try:
        with NamedTemporaryFile(delete=False, suffix=suffix) as temp_file:
            temp_file.write(await file.read())
            temp_path = Path(temp_file.name)
        loaded = load_table_with_metadata(temp_path, sheet_name=sheet)
        return build_cleaning_preview(loaded["frame"], file.filename or temp_path.name)
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc
    finally:
        if "temp_path" in locals() and temp_path.exists():
            temp_path.unlink()
