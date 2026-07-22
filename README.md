# TablePilot

**Local-first messy table workbench: quality scores, analysis plans, chart suggestions.**

[English](README.md) | [中文](README.zh-CN.md)

[![CI](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml/badge.svg)](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

Qt/C++ desktop experience with a Python analysis service.

## Preview

![TablePilot schematic](docs/screenshots/preview.png)

<table>
<tr><td width="50%"><img src="assets/screenshots/tablepilot-desktop-overview.png" alt="TablePilot"></td><td width="50%"><img src="assets/screenshots/tablepilot-chinese-insights.png" alt="TablePilot"></td></tr>
<tr><td width="50%"><img src="assets/screenshots/tablepilot-clean-compare.png" alt="TablePilot"></td><td></td></tr>
</table>

## Features

- Local Excel / CSV / TXT intake
- Schema inference and data-quality scoring
- Analysis planner with insight-oriented cards
- Python analysis_service (FastAPI-oriented)
- Project page: https://phoenix0531-sudo.github.io/TablePilot/

## Get started

### Install

```bash
git clone https://github.com/Phoenix0531-sudo/TablePilot.git
cd TablePilot/analysis_service
pip install -r requirements.txt
```

### Usage

Use demo/ sample tables when present.

```bash
pytest tests/ analysis_service/tests/
```

## Project layout

```
analysis_service/
Statistical_Analysis/
demo/  packaging/  assets/screenshots/
```

## Notes

Not a cloud BI SaaS and not a full Excel replacement.

## License

MIT. Free for commercial use with attribution where applicable. See [LICENSE](LICENSE).
