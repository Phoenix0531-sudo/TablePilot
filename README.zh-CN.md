# TablePilot

**本地优先的脏表工作台 — 质量评分、分析方案、图表建议。**

[English](README.md) | [中文](README.zh-CN.md)

[![CI](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml/badge.svg)](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Python](https://img.shields.io/badge/python-3.10%2B-blue.svg)](https://www.python.org/)

本地优先的脏表工作台 — 质量评分、分析方案、图表建议。

Qt/C++ 桌面体验 + Python 分析服务。


## Screenshots

<table>
<tr><td width="50%"><img src="assets/screenshots/tablepilot-desktop-overview.png" alt="Desktop overview"><br><em>Desktop overview</em></td><td width="50%"><img src="assets/screenshots/tablepilot-chinese-insights.png" alt="Chinese insights"><br><em>Chinese insights</em></td></tr>
<tr><td width="50%"><img src="assets/screenshots/tablepilot-clean-compare.png" alt="Clean compare"><br><em>Clean compare</em></td><td></td></tr>
</table>

## 功能

- 📁 本地 Excel / CSV / TXT 接入
- 🧭 Schema 推断 + 数据质量评分
- 🗂️ 分析规划 + 洞察卡片
- 📊 图表推荐钩子
- 🐍 FastAPI 风格 `analysis_service/`
- 🌐 项目页：https://phoenix0531-sudo.github.io/TablePilot/

## 快速开始

### 安装

```bash
git clone https://github.com/Phoenix0531-sudo/TablePilot.git
cd TablePilot/analysis_service
pip install -r requirements.txt
# desktop build: packaging/ / CMake — see docs/
```

### 使用

可用 `demo/` 样表。服务测试：

```bash
pytest tests/ analysis_service/tests/
```

## 项目结构

```
analysis_service/
Statistical_Analysis/
demo/  packaging/  assets/screenshots/
tests/
```

## 说明

不是云 BI SaaS，也不是完整 Excel 替代品。

## 许可证

MIT。在注明出处的前提下可商业使用（以 LICENSE 为准）。详见 [LICENSE](LICENSE)。
