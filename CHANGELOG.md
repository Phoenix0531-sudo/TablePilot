# Changelog

All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](https://semver.org/).

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [1.0.0] - 2026-06-08

First public release. **Local-first messy-table analysis workbench** — a Python FastAPI
analysis service plus a Qt/C++ desktop shell, processing Excel / CSV / TXT on disk without
uploading.

### Added

- **FastAPI analysis service (`analysis_service/`, v0.5.0)** with 11 live endpoints:
  - `GET /health` — liveness / container probe
  - `GET /api/datasets` — list local data directory
  - `POST /api/analyze` and `POST /api/analyze-upload` — table profiling by name or upload
  - `POST /api/clean-preview-upload` — show what a clean would change
  - `POST /api/clean-upload` — return cleaned CSV or XLSX
  - `POST /api/report/markdown` and `POST /api/report/html` — explainable reports
  - `POST /api/agent/query` — free-text question over a table (optional `local_ai`)
  - `POST /api/session/export` — snapshot the current analysis session as JSON
- **Table profiling** — `profile_dataset` / `profile_table`: schema- and quality-oriented
  profile per table; optional Excel `sheet`; optional `local_ai` enhancement.
- **Cleaning pipeline** — `build_cleaning_preview` (before/after) and `build_cleaned_table`
  (CSV/XLSX output, with a `repair_summary` sheet on XLSX).
- **Reports** — `build_html_report` / `build_markdown_report` produce copy-pasteable,
  evidence-grounded artifacts.
- **Agent narrative** — `answer_question` over a table; `local_ai: true` opts into an
  optional local-model path (e.g. `ollama`-style). Deterministic by default.
- **Qt / C++ desktop shell** (`packaging/`, `qss/`, `Statistical_Analysis/`) driving the
  same service surface over HTTP / JSON.
- **CI** — three GitHub Actions workflows: `CI` (Python 3.11, `pytest`), `Docker
  analysis-service` (container health probe), `Pages` (deploys `site/`).
- **Sample tables** in `demo/` — `quality_issues_demo.csv`, `tablepilot_demo_sales.xlsx`,
  `multi_sheet_operations.xlsx`, `time_series_demo.txt`.
- **Project page** (`site/`) served via GitHub Pages.
- **Real UI captures** in `assets/screenshots/` — desktop overview, Chinese insights,
  clean-compare.

### Changed

- **README repositioned** to its true identity — local-first messy-table workbench —
  with banner, one-line promise, badge matrix (CI / Docker / Pages / MIT / Python / FastAPI
  / Qt / Local-first), TOC anchors, Overview, endpoint-grounded Features, Quickstart command
  table, a mermaid architecture, a real-screenshot proof wall, Scope In/Out, a 3-item FAQ,
  and an AI-assisted + hand-verified method note. Synchronized English (`README.md`) and
  Chinese (`README.zh-CN.md`).
- **Project page (`site/index.html`)** aligned to the same positioning and endpoint
  vocabulary.
- **Visual system** — pure SVG hero (`docs/screenshots/banner.svg`, 1200×280) and a 256×256
  repository avatar (`docs/screenshots/avatar.svg`), derived from a project-native table-grid
  motif and an analysis pipeline (Profile → Clean → Report).

### Notes

- The service component versions independently from the repo: **service `v0.5.0`** lives
  inside the `v1.0.0` repository release. They are intentionally decoupled — the service is
  the contract surface, the repo ships the whole workbench.
- Files are local-only by default; TablePilot never uploads data unless the operator
  explicitly configures a remote destination.
