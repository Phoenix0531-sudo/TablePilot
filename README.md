# TablePilot

**Local-first messy-table workbench — desktop shell + FastAPI analysis service (profile, clean, plan, reports).**

[English](README.md) | [中文](README.zh-CN.md)

[![CI](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml/badge.svg)](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

Hybrid stack: a Qt/C++ oriented desktop experience (see `packaging/`, `qss/`, `Statistical_Analysis/`) talks to a **Python analysis service** that profiles tables and builds analysis artifacts. Local files only by default — not a cloud BI SaaS.

Project page: <https://phoenix0531-sudo.github.io/TablePilot/>

## Preview

![Architecture schematic](docs/screenshots/preview.png)

<table>
<tr>
<td width="50%"><img src="assets/screenshots/tablepilot-desktop-overview.png" alt="Desktop"></td>
<td width="50%"><img src="assets/screenshots/tablepilot-chinese-insights.png" alt="Insights"></td>
</tr>
<tr>
<td width="50%"><img src="assets/screenshots/tablepilot-clean-compare.png" alt="Clean compare"></td>
<td></td>
</tr>
</table>

## Analysis service (real FastAPI surface)

`analysis_service/app/main.py` — **TablePilot Analysis Service v0.5.0**:

Capabilities wired in imports:

- `profile_dataset` / `profile_table` — schema + quality oriented profiling
- `build_cleaning_preview` / `build_cleaned_table` — cleaning preview vs cleaned output
- `build_html_report` / `build_markdown_report` — report generation
- `list_datasets` / `load_table_with_metadata` — local dataset directory
- `answer_question` — optional local-AI narrative path (`local_ai` flag on requests)
- Upload endpoints via FastAPI `UploadFile` for ad-hoc tables

Request model supports `filename` / `dataset` alias, optional Excel `sheet`, optional `local_ai` override.

## Repo layout

```
analysis_service/     # FastAPI service (primary automated surface)
Statistical_Analysis/ # related / legacy analysis modules
demo/                 # sample tables
packaging/            # desktop packaging
assets/screenshots/   # real UI captures
site/                 # project pages
docker-compose.yml
tests/
```

## Install / run service

```bash
git clone https://github.com/Phoenix0531-sudo/TablePilot.git
cd TablePilot/analysis_service
pip install -r requirements.txt
uvicorn app.main:app --reload
```

Desktop build: follow `packaging/` / docs (CMake or project scripts). Use `demo/` tables when present.

```bash
pytest tests/ analysis_service/tests/
```

## Scope

- **In:** local Excel/CSV/TXT profiling, cleaning previews, analysis plans/reports, hybrid desktop+service architecture
- **Out:** multi-tenant cloud warehouse, full Excel formula compatibility

## License

MIT. See [LICENSE](LICENSE).
