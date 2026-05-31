# LatticeIQ

![CI](https://github.com/Phoenix0531-sudo/LatticeIQ/actions/workflows/ci.yml/badge.svg?branch=modern-ai-analysis-workbench)

**中文**：LatticeIQ 是一个本地优先的 AI 表格分析工作台，用 Qt/C++ 提供桌面体验，用 Dockerized FastAPI 提供动态解析、字段识别、质量评分、分析规划和可解释结果。

**English**: LatticeIQ is a local-first AI table analysis workbench. A Qt/C++ desktop client delivers the product experience, while a Dockerized FastAPI service handles dynamic parsing, schema inference, quality scoring, analysis planning, and explainable results.

原始课程/旧版代码保留在 `legacy-qt-statistical-analysis`。现代化版本位于 `modern-ai-analysis-workbench`。

The original legacy version is preserved in `legacy-qt-statistical-analysis`. Modern development lives on `modern-ai-analysis-workbench`.

## Documentation

- [中文文档](docs/README.zh-CN.md)
- [English Documentation](docs/README.en.md)
- [Architecture](docs/ARCHITECTURE.md)

## Highlights

- Excel / CSV / TXT table ingestion.
- Encoding, delimiter, and header inference for messy text tables.
- Semantic schema inference: numeric, date, category, text, empty, high-cardinality.
- Data quality score with missing-value, duplicate, anomaly, sample-size, and analyzability checks.
- Analysis Planner for trend, segment, relationship, anomaly, and quality workflows.
- Qt desktop UI with bilingual Chinese/English switching.
- Local Docker service, GitHub Actions CI, and Windows release packaging script.

## Quick Start

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
samples/latticeiq_demo_sales.xlsx
```

## Release Package

```powershell
powershell -ExecutionPolicy Bypass -File packaging\build-windows-release.ps1
```

Output:

```text
dist/LatticeIQ.zip
```
