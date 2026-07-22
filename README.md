# TablePilot

**Local-first messy table workbench (Qt/C++ desktop + Python analysis service)**

[English](README.md) | [中文](README.zh-CN.md)

![CI](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml/badge.svg)
![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)

TablePilot turns messy Excel / CSV / TXT tables into **explainable profiles**, quality scores, analysis plans, chart suggestions, and exportable reports — **local-first**.

Architecture: Qt/C++ desktop experience + Python analysis service (`analysis_service/`), with optional local LLM (e.g. Ollama) paths for narrative briefs.

Project page: https://phoenix0531-sudo.github.io/TablePilot/

## Why this exists

Business tables arrive dirty. Spreadsheet UIs show cells; they do not ship an analysis plan or quality ledger. TablePilot is a portfolio modernization of an older statistical desktop into a hybrid stack.

## Features

- Local file intake (Excel / CSV / TXT)
- Schema inference and data-quality scoring
- Analysis planner + insight-oriented cards
- Chart recommendation hooks
- Python service under `analysis_service/` (FastAPI-oriented)
- Docker compose for service packaging experiments

## Install / run

```bash
git clone https://github.com/Phoenix0531-sudo/TablePilot.git
cd TablePilot
# analysis service
cd analysis_service && pip install -r requirements.txt
# desktop: build via packaging/ / CMake or scripts documented in docs/
```

Use `demo/` sample tables when available.

## Tests

```bash
pytest tests/ analysis_service/tests/
```

## Project layout

```
analysis_service/
Statistical_Analysis/   # legacy / related analysis modules
demo/
docs/
packaging/
tests/
```

## What this is not

- Not a cloud BI SaaS
- Not a full Excel replacement

## License

MIT. Free for commercial use with attribution. See [LICENSE](LICENSE).
