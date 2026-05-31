# LatticeIQ

![CI](https://github.com/Phoenix0531-sudo/LatticeIQ/actions/workflows/ci.yml/badge.svg?branch=main)

LatticeIQ：本地优先的 AI 表格分析工作台，将 Excel/CSV/TXT 转化为可解释的数据画像、质量评分和分析规划。

Local-first AI table analysis workbench for explainable profiling, quality scoring, and analysis planning over Excel/CSV/TXT files.

## Language

- [中文 README](README.zh-CN.md)
- [English README](README.en.md)

## Demo

- Demo workbook: `demo/latticeiq_demo_sales.xlsx`
- Project page: https://phoenix0531-sudo.github.io/LatticeIQ/

## Quick Start

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

## Release Package

```powershell
powershell -ExecutionPolicy Bypass -File packaging\build-windows-release.ps1
```

Output:

```text
dist/LatticeIQ.zip
```
