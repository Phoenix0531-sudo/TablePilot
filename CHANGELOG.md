# Changelog

All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](https://semver.org/).

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [1.1.0] - 2026-08-12

Internal cleanup, CI tightening, and dependency bumps landed after the public `1.0.0` cut. No breaking service-contract changes; no new user-facing endpoints. Service component version is unchanged (`v0.5.0`) — these are repo-level and CI-level changes.

### Added

- **`.gitattributes`** at repo root — normalizes text files to LF in the object store, pins Windows-only scripts (`*.ps1`/`*.bat`/`*.cmd`) to CRLF, marks image binaries as `binary`, and adds Linguist hints (`.qss` as CSS, `.ui` as XML). Resolves the recurring `LF will be replaced by CRLF` warnings.
- **Ruff lint configuration** in `analysis_service/pyproject.toml` (`[tool.ruff]` + a starter rule set `E, F, I, UP, B, N`) and a new **`Ruff lint (analysis_service)`** job in the CI workflow. Runs on Python 3.11 with `ruff==0.6.9`. The three initially-suppressed rules `I001` / `UP017` / `UP037` were un-ignored in the same window by applying `ruff --fix` to `app/analysis.py`, `app/main.py`, `tests/test_analysis_service.py` (import sorting, `datetime.UTC` alias, de-quoted forward annotation under PEP 563). Ruff now runs fully active with no semantic changes — all 41 tests still pass.
- **`Sync screenshots from assets/`** step in the `Pages` workflow — copies `assets/screenshots/*.png` and the README hero (`docs/screenshots/banner.svg`, `avatar.svg`) into `site/` before the Pages artifact upload, so `site/` no longer carries a second hand-maintained copy of the same bytes.
- **Banner hero on the project page** — `site/index.html` now opens with `<p class="banner"><img src="banner.svg" ...></p>`, reusing the same README hero. New `.banner` CSS in `site/styles.css` (responsive, centered, max-width 960 px).

### Changed

- **`analysis_service/pyproject.toml`** `version` raised `0.4.0 → 0.5.0` to match the runtime `FastAPI(version="0.5.0")` in `app/main.py`. The package metadata is now self-consistent with the running service.
- **GitHub Actions `CI` workflow** — the `Pytest` step no longer appends `|| true` to the `analysis_service/tests` run, so real test regressions now fail CI instead of being silently swallowed.
- **`docs/CHANGELOG.md`** — this section.

### Fixed

- **`analysis_service/tests/test_endpoints_extra.py`** — corrected three `/api/agent/query` intent assertions that had become latent regressions hidden by the previous `|| true`:
  - `data_quality` case question no longer triggers the `anomaly_review` route (the "风险" keyword was being matched first).
  - `correlation_review` and `dataset_overview` cases now assert against the real `profile_dataset` tool trace (`load_table` / `infer_schema`-class steps) instead of the `plan.tools` placeholder names (`calculate_correlations` / `profile_dataset`), which `answer_question` does not surface verbatim in `tools_used`.
- All 41 tests pass under the new dependencies below.

### Dependencies bumped (via Dependabot, squash-merged to `main`)

| Package / Action | From | To |
| --- | --- | --- |
| `fastapi` | `0.115.6` | `0.141.1` |
| `uvicorn[standard]` | `0.34.0` | `0.52.1` |
| `pandas` | `2.2.3` | `3.0.5` |
| `xlrd` | `2.0.1` | `2.0.2` |
| `python-multipart` | `0.0.20` | `0.0.32` |
| `actions/checkout` | `v4` | `v7` |
| `actions/configure-pages` | `v5` | `v6` |
| `actions/upload-pages-artifact` | `v3` | `v5` |

Eight open Dependabot PRs were closed in this window; `main` is current with no outstanding bot PRs.

### Removed

- **`site/screenshots/*.png`** — three duplicate copies of `assets/screenshots/` content (verified identical by SHA-256). Now built in CI by the `Sync screenshots from assets/` step.

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
