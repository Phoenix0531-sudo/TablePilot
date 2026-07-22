# TablePilot

**本地优先脏表工作台 — 桌面壳 + FastAPI 分析服务（画像、清洗、方案、报告）。**

[English](README.md) | [中文](README.zh-CN.md)

[![CI](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml/badge.svg)](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

混合架构：Qt/C++ 向桌面体验（`packaging/`、`qss/`、`Statistical_Analysis/`）对接 **Python 分析服务**。默认本地文件，不是云 BI。

项目页：<https://phoenix0531-sudo.github.io/TablePilot/>

## 预览

![架构示意](docs/screenshots/preview.png)

（真实界面截图见 `assets/screenshots/`）

## 分析服务

`analysis_service/app/main.py`（v0.5.0）：表画像、清洗预览/结果、HTML/Markdown 报告、本地数据集列表加载、上传、可选 `local_ai` 叙事。

## 安装

```bash
cd analysis_service
pip install -r requirements.txt
uvicorn app.main:app --reload
pytest tests/ analysis_service/tests/
```

桌面构建见 `packaging/` 与文档。

## 许可证

MIT。详见 [LICENSE](LICENSE)。
