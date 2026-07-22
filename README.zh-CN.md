# TablePilot

**本地优先的脏表工作台（Qt/C++ 桌面 + Python 分析服务）**

[English](README.md) | [中文](README.zh-CN.md)

![CI](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml/badge.svg)
![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)

TablePilot 把杂乱的 Excel / CSV / TXT 变成**可解释的数据画像**、质量分、分析计划、图表建议与可导出报告——**本地优先**。

架构：Qt/C++ 桌面体验 + Python 分析服务（`analysis_service/`），可选本地 LLM（如 Ollama）生成叙述性摘要。

项目页：https://phoenix0531-sudo.github.io/TablePilot/

## 为什么做这个

业务表往往很脏。表格软件只显示单元格，不给出分析计划与质量账本。TablePilot 是把旧统计桌面现代化为混合栈的作品集项目。

## 功能

- 本地文件接入  
- Schema 推断与数据质量评分  
- 分析规划与 insight 卡片  
- 图表推荐钩子  
- `analysis_service/` Python 服务  
- Docker compose 实验  

## 安装 / 运行

```bash
git clone https://github.com/Phoenix0531-sudo/TablePilot.git
cd TablePilot
cd analysis_service && pip install -r requirements.txt
# 桌面端见 packaging/ 与 docs/
```

可用 `demo/` 样例表。

## 测试

```bash
pytest tests/ analysis_service/tests/
```

## 目录结构

```
analysis_service/
Statistical_Analysis/
demo/
docs/
packaging/
tests/
```

## 明确不做

- 非云端 BI SaaS  
- 非完整 Excel 替代品  

## 许可证

MIT。可在署名前提下商用。见 [LICENSE](LICENSE)。
