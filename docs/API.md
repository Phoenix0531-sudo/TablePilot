# TablePilot Analysis Service — API Quick Reference

The TablePilot analysis service (`analysis_service/`, **v0.5.0**) runs locally and exposes
**11 HTTP endpoints** over FastAPI. Every capability below is reachable over plain HTTP; the
auto-generated Swagger UI is at `http://127.0.0.1:8000/docs` once the service is running.

> Run the service:
>
> ```bash
> cd analysis_service
> pip install -r requirements.txt
> uvicorn app.main:app --reload
> ```
>
> The local data directory defaults to `../demo` (overridable with the `DATA_DIR` env var).
> Sample tables ship in `demo/` — see [Sample tables](#sample-tables) below.

> **OpenAPI schema** — the live spec is served at `http://127.0.0.1:8000/openapi.json`
> (FastAPI default). To freeze a snapshot into the repo for review, run
> `python scripts/dump_openapi.py`, which writes `docs/openapi.json` from the exact
> `app.main:app` instance described here. CI re-runs that script on every change as
> a contract guard. The Swagger UI (`/docs`) and ReDoc (`/redoc`) are both served by
> the same app — no separate build step.

---

## Request model

Several endpoints accept the same JSON body shape, derived from `DatasetRequest` in
`analysis_service/app/main.py`:

| Field        | Type              | Required                                     | Notes                                            |
| ------------ | ----------------- | -------------------------------------------- | ------------------------------------------------ |
| `filename`   | `string`          | yes (or use `dataset`)                       | Dataset filename under the data directory.       |
| `dataset`    | `string`          | alias for `filename`                         | Either `filename` or `dataset` must be present. |
| `sheet`      | `string \| null`  | no                                           | Optional Excel sheet name.                       |
| `local_ai`   | `bool \| null`    | no                                           | Override optional local-model enhancement.       |

Endpoints named `*-upload` instead take a `multipart/form-data` file upload (with optional
`sheet` and `local_ai` / `format` form fields) rather than the JSON body above.

---

## Endpoints

### `GET /health`

Service liveness probe. Used by CI Docker health checks and container orchestrators.

```bash
curl http://127.0.0.1:8000/health
# -> { "status": "ok" }
```

### `GET /api/datasets`

List every analyzable file in the configured local data directory.

```bash
curl http://127.0.0.1:8000/api/datasets
# -> { "datasets": [ { "filename": "...", "extension": ".csv", "size_bytes": 1234 }, ... ] }
```

### `POST /api/analyze` — profile a dataset by name

Run the full profiling pipeline (`profile_dataset`) on a dataset that lives in the data
directory. Returns schema, quality, anomalies, correlations, trends, recommended views,
insight cards, decision brief, executive brief, quality-repair plan, and the optional
`local_ai` block (non-circular — together, one of the richest JSON shapes in the service).

| Field       | Required             | Notes                                   |
| ----------- | -------------------- | --------------------------------------- |
| `filename`  | yes (or `dataset`)   | file must exist in the data directory.  |
| `sheet`     | no                   | Excel sheet name.                       |
| `local_ai`  | no                   | opt into optional local-model narration.|

```bash
curl -X POST http://127.0.0.1:8000/api/analyze \
  -H 'Content-Type: application/json' \
  -d '{"filename":"quality_issues_demo.csv"}'
```

Top-level response keys (see `profile_table` in `analysis_service/app/analysis.py`):

```
session            dataset            source              table_diagnostics
schema             quality            columns             anomalies
correlations       trends             chart_recommendations
recommended_views  analysis_recommendations
analysis_plan     business_analysis  dataset_fingerprint
insight_cards     decision_brief     quality_repair_plan
executive_brief   preview            tool_trace          insights
local_ai
```

Each `dataset` entry carries `{filename, extension, rows, columns, numeric_columns,
date_columns, category_columns, missing_cells}`. `quality` carries `{score, level,
missing_ratio, duplicate_rows, anomaly_count, ...}` (level ∈ `high | medium | low`).

### `POST /api/analyze-upload` — profile an uploaded file

Same pipeline as `/api/analyze`, but the table is sent as `multipart/form-data`. Supports
the file types surfaced in `analysis.py`: `.xlsx`, `.xls`, `.csv`, `.txt`. Optional `sheet`
selects an Excel sheet; optional `local_ai` overrides local-AI wording.

```bash
curl -X POST http://127.0.0.1:8000/api/analyze-upload \
  -F "file=@demo/quality_issues_demo.csv"
```

Response shape is identical to `/api/analyze`.

### `POST /api/clean-preview-upload` — preview *what would change*

Returns a **before/after** preview without writing a cleaned file. See `build_cleaning_preview`
in `analysis_service/app/analysis.py`.

```bash
curl -X POST http://127.0.0.1:8000/api/clean-preview-upload \
  -F "file=@demo/quality_issues_demo.csv" | jq
```

Response keys:

```
summary    {filename, original_rows, original_columns, cleaned_rows, cleaned_columns,
             removed_empty_rows, removed_empty_columns, removed_duplicate_rows,
             filled_missing_cells, marked_anomaly_rows, repairs[]}
before     {headers[], rows[][]}
after      {headers[], rows[][]}
```

### `POST /api/clean-upload` — return the cleaned table

Applies conservative repairs (drop empty rows/columns, drop duplicates, fill numeric with
median, fill text/category with mode, mark anomalies rather than delete them) and streams
the cleaned table back. The `format` form field selects the output (`csv` default, or
`xlsx`). XLSX output adds a `repair_summary` sheet alongside `cleaned`.

```bash
# CSV output
curl -X POST http://127.0.0.1:8000/api/clean-upload \
  -F "file=@demo/quality_issues_demo.csv" -F "format=csv" \
  -o quality_issues-cleaned.csv

# XLSX output (with a repair_summary sheet)
curl -X POST http://127.0.0.1:8000/api/clean-upload \
  -F "file=@demo/multi_sheet_operations.xlsx" -F "format=xlsx" \
  -o multi_sheet-cleaned.xlsx
```

### `POST /api/agent/query` — deterministic question agent

Answers a free-text question over a table using a deterministic, evidence-grounded agent
(`answer_question` in `analysis_service/app/agent.py`). Intent is routed off keywords in the
question (anomaly / quality / trend / correlation / overview). Local-AI wording is
intentionally **disabled** here by default — `llm_status: "disabled"` is returned — but the
shape is documented so a future Ollama / OpenAI-compatible path can slot in.

| Field       | Required             |
| ----------- | -------------------- |
| `filename`  | yes (or `dataset`)   |
| `question`  | yes, non-empty       |
| `sheet`     | no                   |

```bash
curl -X POST http://127.0.0.1:8000/api/agent/query \
  -H 'Content-Type: application/json' \
  -d '{"filename":"tablepilot_demo_sales.xlsx","question":"有哪些异常值需要复核？"}'
```

Response keys:

```
question    intent        tools_used    tool_trace
answer      evidence { dataset, source, schema, quality,
              analysis_plan, recommendations, chart_recommendations,
              top_anomalies, top_correlations, top_trends }
llm_status  note
```

### `POST /api/report/markdown` — generate a Markdown report

Returns an explainable Markdown report derived from the same profile as `/api/analyze`
(`build_markdown_report`). `response_class=PlainTextResponse`. Body shape is
`{filename, sheet?, local_ai?}` (the analyze request model).

```bash
curl -X POST http://127.0.0.1:8000/api/report/markdown \
  -H 'Content-Type: application/json' \
  -d '{"filename":"quality_issues_demo.csv"}' \
  -o quality_issues-report.md
```

### `POST /api/report/html` — generate an HTML report

Same as above but `response_class=HTMLResponse` (`build_html_report`).

```bash
curl -X POST http://127.0.0.1:8000/api/report/html \
  -H 'Content-Type: application/json' \
  -d '{"filename":"quality_issues_demo.csv"}' \
  -o quality_issues-report.html
```

### `POST /api/session/export` — snapshot the analysis session

Returns the current profile under an explicit `session` envelope. Same body as analyze.
Useful for archiving an analysis run.

```bash
curl -X POST http://127.0.0.1:8000/api/session/export \
  -H 'Content-Type: application/json' \
  -d '{"filename":"quality_issues_demo.csv"}'
# -> { "session": { id, generated_at, report_formats }, "profile": { ...full profile } }
```

---

## Sample tables

Four sample tables ship in `demo/`. They are the fastest way to exercise every endpoint.

| File                                | Type    | Best for                                                    |
| ----------------------------------- | ------- | ---------------------------------------------------------- |
| `quality_issues_demo.csv`           | CSV     | First run. Clean-quality demo with missing values and anomalies. Pairs well with `/api/clean-preview-upload` and `/api/clean-upload`. |
| `tablepilot_demo_sales.xlsx`        | XLSX    | Segment / trend / correlation review. Great target for `/api/agent/query`. |
| `multi_sheet_operations.xlsx`       | XLSX    | Excel sheet selection. Pass `sheet` to pick a sheet when calling `/api/analyze` or `/api/clean-upload?format=xlsx`. |
| `time_series_demo.txt`             | TXT     | Encoding/delimiter autodetection (tab-comma-whitespace fallback). Exercises the text-parsing branch of `load_table_with_metadata`. |

### Suggested walkthrough

1. `GET /api/datasets` — confirm the data directory resolved and lists all four files.
2. `POST /api/analyze` with `quality_issues_demo.csv` — read `quality.score`, `insight_cards`,
   and `quality_repair_plan`.
3. `POST /api/clean-preview-upload` with the same file — compare `before` and `after` previews.
4. `POST /api/clean-upload` — stream the cleaned CSV (or `format=xlsx` for the `repair_summary` sheet).
5. `POST /api/agent/query` with `tablepilot_demo_sales.xlsx` and a Chinese question such as
   `有哪些异常值需要复核？` to exercise the keyword-routed agent.
6. `POST /api/report/markdown` (or `/html`) — save the explainable report to disk.

---

## Environment variables

The service reads a few env vars (see `analysis_service/app/analysis.py` `build_local_ai_enhancement`):

| Variable                            | Default                            | Effect                                                    |
| ----------------------------------- | ---------------------------------- | --------------------------------------------------------- |
| `DATA_DIR`                          | `<repo>/demo`                      | Local data directory scanned by `/api/datasets`.          |
| `TABLEPILOT_ENABLE_LOCAL_AI`        | unset → disabled                   | `1`/`true`/`yes` enables local-AI wording in profiles.    |
| `TABLEPILOT_ENABLE_OLLAMA`          | unset → disabled                   | Alias for enabling the local-AI path.                     |
| `TABLEPILOT_LOCAL_AI_PROVIDER`      | `ollama` (or `openai-compatible`)   | Selects the wording backend.                              |
| `LOCAL_LLM_BASE_URL`                | unset                              | OpenAI-compatible endpoint base URL when applicable.      |
| `LOCAL_LLM_MODEL` / `OLLAMA_MODEL`  | `qwen2.5:1.5b`                     | Model name passed to the local-AI backend.                |
| `OLLAMA_URL`                        | `http://[IP]:11434/api/generate`   | Ollama generation endpoint when provider is `ollama`.     |

When local-AI is disabled, the `local_ai` block in the profile is
`{provider, model, status:"disabled", summary:null, guardrail:"..."}` — deterministic
evidence is used. Nothing leaves your machine unless you point these at a remote host.
