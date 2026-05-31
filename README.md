# TablePilot

[![CI](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml)
[![Project Page](https://img.shields.io/badge/project-page-111111)](https://phoenix0531-sudo.github.io/TablePilot/)

[English](README.md) | [中文](README.zh-CN.md)

TablePilot is a local-first desktop workbench for turning Excel, CSV, and TXT tables into explainable data profiles, quality scores, analysis plans, and visual summaries.

It modernizes an older Qt statistical analysis project into a hybrid C++/Python portfolio project: the desktop experience is built with Qt/C++, while a Dockerized FastAPI service handles parsing, profiling, planning, report generation, and API-level tests.

## Why This Project Exists

Most small spreadsheet utilities assume a fixed table shape or a single demo file. TablePilot is designed for messier real-world files:

- different row and column counts
- Excel, CSV, and TXT inputs
- delimiter detection for text-like tables
- schema inference instead of hard-coded columns
- quality scoring before analysis
- explainable analysis planning instead of opaque output
- Chinese/English desktop UI

## Product Name

`TablePilot` means a co-pilot for tabular data. It does not try to replace a human analyst; it helps users load unfamiliar tables, understand their structure, spot quality risks, and choose the next useful analysis step.

## Highlights

- **Dynamic table parsing** for Excel, CSV, and TXT files.
- **Schema inference** for numeric, date, category, text, empty, and high-cardinality fields.
- **Data quality scoring** using missing values, duplicates, anomalies, sample size, and analyzability.
- **Analysis Planner** that recommends next steps based on schema, trends, correlations, and anomalies.
- **Desktop-native Qt UI** with dynamic preview tables, charts, profile cards, and a bilingual insight panel.
- **Local-first architecture**: the Python analysis service runs locally through Docker.
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
      +--> Parser and schema inference
      +--> Quality scoring
      +--> Trends, correlations, anomalies
      +--> Analysis planner
      +--> Markdown report and agent-style answers
```

## Repository Structure

```text
Statistical_Analysis/          Qt/C++ desktop client
analysis_service/              FastAPI analysis service
analysis_service/tests/        Backend tests and fixtures
config/                        Product config and bilingual UI copy
demo/                          Demo workbook for manual review
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

Demo workbook:

```text
demo/tablepilot_demo_sales.xlsx
```

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
- Add user-selectable chart dimensions and chart types.
- Add structured report export from the desktop UI.
- Add screenshot-backed project page after final UI polish.
- Add GitHub Release automation for Windows packages.

## Disclaimer

TablePilot produces analytical summaries for learning and portfolio demonstration. It is not a business, financial, or operational recommendation system.
