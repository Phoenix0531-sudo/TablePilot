# TablePilot

**本地优先的脏表工作台：质量评分、分析方案、图表建议。**

[English](README.md) | [中文](README.zh-CN.md)

[![CI](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml/badge.svg)](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

Qt/C++ 桌面体验 + Python 分析服务。

## 预览

![架构示意](docs/screenshots/preview.png)

<table>
<tr><td width="50%"><img src="assets/screenshots/tablepilot-desktop-overview.png" alt="TablePilot"></td><td width="50%"><img src="assets/screenshots/tablepilot-chinese-insights.png" alt="TablePilot"></td></tr>
<tr><td width="50%"><img src="assets/screenshots/tablepilot-clean-compare.png" alt="TablePilot"></td><td></td></tr>
</table>

## 功能

- 本地 Excel / CSV / TXT 接入
- Schema 推断与数据质量评分
- 分析规划与洞察卡片
- Python analysis_service（FastAPI 风格）
- 项目页：https://phoenix0531-sudo.github.io/TablePilot/

## 快速开始

### 安装

```bash
git clone https://github.com/Phoenix0531-sudo/TablePilot.git
cd TablePilot/analysis_service
pip install -r requirements.txt
```

### 使用

可用 demo/ 样表。

```bash
pytest tests/ analysis_service/tests/
```

## 项目结构

```
analysis_service/
Statistical_Analysis/
demo/  packaging/  assets/screenshots/
```

## 说明

不是云 BI SaaS，也不是完整 Excel 替代品。

## 许可证

MIT。在注明出处的前提下可商业使用（以 LICENSE 为准）。详见 [LICENSE](LICENSE)。
