# TablePilot

**Local-first messy table workbench — quality scores, analysis plans, chart suggestions.**

[English](README.md) | [中文](README.zh-CN.md)

[![CI](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml/badge.svg)](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Python](https://img.shields.io/badge/python-3.10%2B-blue.svg)](https://www.python.org/)

Local-first messy table workbench — quality scores, analysis plans, chart suggestions.

Qt/C++ desktop experience + Python analysis service.


## Screenshots

<table>
<tr><td width="50%"><img src="assets/screenshots/tablepilot-desktop-overview.png" alt="Desktop overview"><br><em>Desktop overview</em></td><td width="50%"><img src="assets/screenshots/tablepilot-chinese-insights.png" alt="Chinese insights"><br><em>Chinese insights</em></td></tr>
<tr><td width="50%"><img src="assets/screenshots/tablepilot-clean-compare.png" alt="Clean compare"><br><em>Clean compare</em></td><td></td></tr>
</table>

## Features

- 📁 Local Excel / CSV / TXT intake
- 🧭 Schema inference + data-quality scoring
- 🗂️ Analysis planner + insight-oriented cards
- 📊 Chart recommendation hooks
- 🐍 FastAPI-oriented `analysis_service/`
- 🌐 Project page: https://phoenix0531-sudo.github.io/TablePilot/

## Get started

### Install

```bash
git clone https://github.com/Phoenix0531-sudo/TablePilot.git
cd TablePilot/analysis_service
pip install -r requirements.txt
# desktop build: packaging/ / CMake — see docs/
```

### Usage

Use `demo/` sample tables when present. Run service tests:

```bash
pytest tests/ analysis_service/tests/
```

## Project layout

```
analysis_service/
Statistical_Analysis/
demo/  packaging/  assets/screenshots/
tests/
```

## Notes

Not a cloud BI SaaS and not a full Excel replacement.

## License

MIT. Free for commercial use with attribution where applicable. See [LICENSE](LICENSE).
