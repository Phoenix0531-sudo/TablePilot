# TablePilot

[![CI](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml)
[![Project Page](https://img.shields.io/badge/project-page-111111)](https://phoenix0531-sudo.github.io/TablePilot/)

[English](README.md) | [中文](README.zh-CN.md)

TablePilot is a local-first desktop workbench for turning messy Excel, CSV, and TXT tables into explainable data profiles, quality scores, analysis plans, chart recommendations, and exportable reports.

It modernizes an older Qt statistical analysis project into a hybrid C++/Python portfolio project: the desktop experience is built with Qt/C++, while a Dockerized FastAPI service handles parsing, profiling, planning, report generation, and API-level tests.

## Why This Project Exists

Most small spreadsheet utilities assume a fixed table shape or a single demo file. TablePilot is designed for messier real-world files:

- different row and column counts
- Excel workbooks with multiple sheets
- CSV/TXT files with comma, tab, semicolon, pipe, or whitespace delimiters
- non-first-row headers, empty rows, empty columns, duplicate fields, and total rows
- schema inference instead of hard-coded columns
- quality scoring and repair planning before analysis
- explainable analysis planning instead of opaque output
- optional local LLM wording layer with deterministic evidence as the source of truth
- Chinese/English desktop UI

## Product Name

`TablePilot` means a co-pilot for tabular data. It does not try to replace a human analyst; it helps users load unfamiliar tables, understand their structure, spot quality risks, and choose the next useful analysis step.

## Highlights

- **Messy Table Autopilot** for Excel, CSV, and TXT files, including delimiter, encoding, header-row, empty-structure, summary-row, and multi-sheet handling.
- **Schema inference** for numeric, date, category, text, empty, and high-cardinality fields.
- **Data Quality Repair Plan + Clean Export** using missing values, duplicates, duplicate fields, anomalies, sample size, and analyzability, with conservative cleaned CSV/XLSX export.
- **Analysis Planner** that recommends next steps based on schema roles, trends, correlations, anomalies, and quality risks.
- **Insight Cards** that package findings into user-facing cards with evidence and suggested actions.
- **Dynamic Chart Studio** with auto chart selection, grouped bars, scatter plots, correlation heatmaps, box plots, metric/dimension selectors, generated subtitles, and professional empty states.
- **Session and Report System** with profile IDs, generation time, Markdown reports, HTML reports, and desktop report export.
- **Desktop-native Qt UI** with dynamic preview tables, sheet switching, charts, profile cards, and a bilingual insight panel.
- **Local-first architecture**: the Python analysis service runs locally through Docker.
- **Optional local LLM support**: Ollama and OpenAI-compatible llama.cpp endpoints can be enabled from the desktop UI without making the LLM authoritative.
- **Deterministic agent-style API** for question answering over loaded datasets.
- **CI coverage** for parser behavior, API endpoints, Docker build, and package metadata.
- **Windows release script** for building a distributable desktop package.

## Architecture

```text
Excel / CSV / TXT
      |
      v
Qt/C++ Desktop Client
      |
      v
FastAPI Analysis Service
      |
      +--> Messy table parser and sheet selector
      +--> Schema inference and semantic roles
      +--> Quality scoring, repair plan, and cleaned export
      +--> Trends, correlations, anomalies
      +--> Insight cards and analysis planner
      +--> Markdown / HTML reports and agent-style answers
      +--> Optional local LLM wording layer
```

## Repository Structure

```text
Statistical_Analysis/          Qt/C++ desktop client
analysis_service/              FastAPI analysis service
analysis_service/tests/        Backend tests and fixtures
config/                        Product config and bilingual UI copy
demo/                          Demo workbook, messy CSV, and time-series TXT files
packaging/                     Windows release scripts
site/                          GitHub Pages showcase
qss/                           Desktop theme
```

## Quick Start

Start the local analysis service:

```powershell
docker compose up --build
```

Open the desktop project with Qt Creator:

```text
Statistical_Analysis/Statistical_Analysis.pro
```

Recommended kit:

```text
Qt 6.11.1 MinGW 64-bit
```

Demo files:

```text
demo/tablepilot_demo_sales.xlsx
demo/multi_sheet_operations.xlsx
demo/quality_issues_demo.csv
demo/time_series_demo.txt
```

`tablepilot_demo_sales.xlsx` is the primary happy-path workbook. `multi_sheet_operations.xlsx` validates workbook sheet switching. `quality_issues_demo.csv` is intentionally messy and exercises the repair-plan flow. `time_series_demo.txt` validates whitespace-delimited TXT parsing and trend planning.

## API Smoke Test

```powershell
Invoke-RestMethod http://127.0.0.1:8000/health
```

```powershell
Invoke-RestMethod `
  -Method Post `
  -Uri http://127.0.0.1:8000/api/analyze `
  -ContentType "application/json" `
  -Body '{"filename":"tablepilot_demo_sales.xlsx"}'
```

## Tests

```powershell
python -m pip install -r analysis_service/requirements-dev.txt
$env:PYTHONPATH = "$PWD\analysis_service"
python -m pytest -q analysis_service\tests
```

If Windows blocks pytest's default temp directory, use a project-local temp folder:

```powershell
New-Item -ItemType Directory -Force .tmp | Out-Null
python -m pytest -q analysis_service\tests --basetemp .tmp\pytest
```

## Local AI

The analysis engine is deterministic by default. A local model is optional and only rewrites evidence-grounded summaries.

OpenAI-compatible local llama.cpp endpoint:

```powershell
$env:TABLEPILOT_ENABLE_LOCAL_AI = "1"
$env:TABLEPILOT_LOCAL_AI_PROVIDER = "openai-compatible"
$env:LOCAL_LLM_BASE_URL = "http://127.0.0.1:39281/v1"
$env:LOCAL_LLM_MODEL = "qwen3-4b"
docker compose up --build
```

Model smoke test:

```powershell
Invoke-RestMethod "http://127.0.0.1:39281/v1/models"
```

Ollama remains supported:

```powershell
$env:TABLEPILOT_ENABLE_LOCAL_AI = "1"
$env:TABLEPILOT_LOCAL_AI_PROVIDER = "ollama"
$env:OLLAMA_MODEL = "qwen2.5:1.5b"
$env:OLLAMA_URL = "http://127.0.0.1:11434/api/generate"
docker compose up --build
```

If the local model is disabled, unavailable, or fails the evidence guardrail, TablePilot continues to run with deterministic analysis and reports the local AI status in the response. Model text is suppressed when it references fields that are not present in the structured profile.

## Clean Export API

```powershell
curl.exe -F "file=@demo/quality_issues_demo.csv;type=text/csv" `
  "http://127.0.0.1:8000/api/clean-upload?format=csv" `
  -o quality_issues_demo-cleaned.csv
```

Use `format=xlsx` to export a workbook with a `cleaned` sheet and a `repair_summary` sheet.

## Report and Session APIs

```powershell
Invoke-RestMethod `
  -Method Post `
  -Uri http://127.0.0.1:8000/api/report/markdown `
  -ContentType "application/json" `
  -Body '{"filename":"tablepilot_demo_sales.xlsx"}'
```

```powershell
Invoke-RestMethod `
  -Method Post `
  -Uri http://127.0.0.1:8000/api/session/export `
  -ContentType "application/json" `
  -Body '{"filename":"tablepilot_demo_sales.xlsx"}'
```

## Windows Release Package

```powershell
powershell -ExecutionPolicy Bypass -File packaging\build-windows-release.ps1
```

Output:

```text
dist/TablePilot.zip
```

The package includes the Qt executable, Docker Compose file, analysis service, demo data, config files, and release notes.

## Configuration and Localization

Product-level settings live in:

```text
config/app.json
```

Bilingual copy lives in:

```text
config/i18n.en.json
config/i18n.zh-CN.json
```

The current desktop UI already supports Chinese/English switching. These config files make the project easier to maintain as the UI copy grows.

## Branches

- `main`: current modern TablePilot version.
- `legacy-original-qt`: preserved original Qt statistical analysis version.

## Roadmap

- Add drag-and-drop welcome screen and recent files.
- Add chart image export from each Chart Studio view.
- Add packaged local-model smoke-test mode for OpenAI-compatible and Ollama endpoints.
- Add screenshot-backed project page after final UI polish.
- Add GitHub Release automation for Windows packages.

## Disclaimer

TablePilot produces analytical summaries for learning and portfolio demonstration. It is not a business, financial, or operational recommendation system.
