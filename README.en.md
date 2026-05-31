# LatticeIQ English README

LatticeIQ is a local-first AI table analysis workbench. The Qt/C++ desktop client provides the product experience, while a Dockerized FastAPI service handles dynamic parsing, schema inference, data quality scoring, analysis planning, and explainable output.

## What the Name Means

`Lattice` refers to a grid or structured network, which fits tabular data, field relationships, and schema structure. `IQ` signals intelligence and analysis capability.  
`LatticeIQ` means: **discover the structure inside messy tables and turn it into explainable intelligent analysis.**

## Highlights

- Supports Excel, CSV, and TXT table files.
- Detects encoding, delimiter, and header rows for TXT/CSV files.
- Infers semantic field types: numeric, date, category, text, empty, and high-cardinality.
- Scores data quality from missing values, duplicate rows, anomalies, sample size, and analyzability.
- Includes an Analysis Planner that recommends next steps from schema, quality, trends, correlations, and anomalies.
- Provides Chinese/English switching in the Qt desktop UI.
- Uses a Dockerized local analysis service to keep the desktop app clean.
- Includes GitHub Actions CI and a Windows zip release packaging script.
- Includes a GitHub Pages project showcase.

## Structure

```text
Statistical_Analysis/          Qt/C++ desktop client
analysis_service/              FastAPI analysis service
analysis_service/tests/        Backend tests and fixtures
demo/                          Manual demo workbook
packaging/                     Windows release scripts
site/                          GitHub Pages showcase
qss/                           Desktop theme
```

## Quick Start

Start the local analysis service:

```powershell
docker compose up --build
```

Open the Qt project:

```text
Statistical_Analysis/Statistical_Analysis.pro
```

Recommended kit:

```text
Qt 6.11.1 MinGW 64-bit
```

Demo workbook:

```text
demo/latticeiq_demo_sales.xlsx
```

## Release Package

```powershell
powershell -ExecutionPolicy Bypass -File packaging\build-windows-release.ps1
```

Output:

```text
dist/LatticeIQ.zip
```

## Branches

- `main`: current modern version.
- `legacy-original-qt`: preserved original Qt statistical analysis version.
