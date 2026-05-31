# TablePilot

[![CI](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml)
[![项目主页](https://img.shields.io/badge/project-page-111111)](https://phoenix0531-sudo.github.io/TablePilot/)

[English](README.md) | [中文](README.zh-CN.md)

TablePilot 是一个本地优先的桌面数据分析工作台，可以把混乱的 Excel、CSV 和 TXT 表格转化为可解释的数据画像、质量评分、分析规划、图表建议和可导出的分析报告。

这个项目把一个早期 Qt 统计分析程序升级成了 C++/Python 混合的作品集项目：桌面端由 Qt/C++ 提供，动态解析、字段识别、质量评分、分析规划、报告生成和接口测试由 Dockerized FastAPI 服务完成。

## 为什么做这个项目

很多小型表格工具只适合固定格式或单一演示文件。TablePilot 面向更真实的表格场景：

- 行列数量不固定
- Excel 多工作表
- CSV/TXT 里逗号、Tab、分号、竖线、空格分隔混用
- 表头不在第一行、空行空列、重复字段、合计行、备注行
- 自动推断字段结构，而不是写死列名
- 分析前先做数据质量评分和修复计划
- 用可解释的分析规划替代黑盒输出
- 可选本地模型润色，但结构化证据始终是事实来源
- 桌面端支持中英文切换

## 名字是什么意思

`TablePilot` 的意思是“表格数据驾驶员 / 表格分析助手”。它不是替代分析师，而是帮助用户快速加载陌生表格、理解字段结构、发现质量风险，并选择下一步最值得做的分析。

## 项目亮点

- **Messy Table Autopilot**：支持 Excel、CSV、TXT，并自动识别分隔符、编码、表头行、空结构、合计行和多工作表。
- **字段结构识别**：numeric、date、category、text、empty、high-cardinality。
- **数据质量修复计划 + 清洗导出**：综合缺失值、重复行、重复字段、异常值、样本量和可分析性，给出影响范围、修复建议，并支持保守清洗后的 CSV/XLSX 导出。
- **Analysis Planner**：根据字段角色、趋势、相关性、异常和质量风险自动推荐下一步分析。
- **Insight Cards**：把关键发现整理成面向用户的洞察卡片，包含结论、证据和建议动作。
- **Dynamic Chart Studio**：自动推荐图表，支持分组柱状图、散点图、相关性热力图、箱线图、指标/维度双选择器、图表副标题和专业空状态。
- **Session / Report System**：记录 profile ID、生成时间、Markdown 报告、HTML 报告，并支持桌面端导出。
- **Qt 桌面端体验**：动态预览表格、工作表切换、图表、画像卡片和双语洞察面板。
- **本地优先架构**：Python 分析服务通过 Docker 在本地运行。
- **可选本地模型支持**：支持 Ollama 和 OpenAI-compatible llama.cpp 接口，可从桌面端请求本地模型增强；本地模型只负责表达增强，不负责创造事实。
- **确定性 Agent 风格接口**：可以围绕加载的数据集做问题回答。
- **CI 覆盖**：解析逻辑、API、Docker build 和包元数据。
- **Windows 发布脚本**：可以构建桌面端 zip 发布包。

## 架构

```text
Excel / CSV / TXT
      |
      v
Qt/C++ 桌面客户端
      |
      v
FastAPI 分析服务
      |
      +--> 混乱表格解析和工作表选择
      +--> 字段识别和语义角色
      +--> 数据质量评分、修复计划和清洗导出
      +--> 趋势、相关性、异常
      +--> 洞察卡片和分析规划器
      +--> Markdown / HTML 报告和 Agent 风格回答
      +--> 可选本地模型表达层
```

## 目录结构

```text
Statistical_Analysis/          Qt/C++ 桌面端
analysis_service/              FastAPI 分析服务
analysis_service/tests/        后端测试与测试夹具
config/                        产品配置和中英文文案
demo/                          演示 Excel、质量问题 CSV、时间序列 TXT
packaging/                     Windows 发布脚本
site/                          GitHub Pages 展示页
qss/                           桌面端主题
```

## 快速运行

启动本地分析服务：

```powershell
docker compose up --build
```

用 Qt Creator 打开：

```text
Statistical_Analysis/Statistical_Analysis.pro
```

推荐 Kit：

```text
Qt 6.11.1 MinGW 64-bit
```

演示文件：

```text
demo/tablepilot_demo_sales.xlsx
demo/multi_sheet_operations.xlsx
demo/quality_issues_demo.csv
demo/time_series_demo.txt
```

`tablepilot_demo_sales.xlsx` 是主流程演示数据。`multi_sheet_operations.xlsx` 用于验证 Excel 多工作表切换。`quality_issues_demo.csv` 故意保留缺失、重复和异常，用于演示修复计划。`time_series_demo.txt` 用于验证空格分隔 TXT 解析和趋势规划。

## API 冒烟测试

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

## 测试

```powershell
python -m pip install -r analysis_service/requirements-dev.txt
$env:PYTHONPATH = "$PWD\analysis_service"
python -m pytest -q analysis_service\tests
```

如果 Windows 默认临时目录权限异常，可以使用项目内临时目录：

```powershell
New-Item -ItemType Directory -Force .tmp | Out-Null
python -m pytest -q analysis_service\tests --basetemp .tmp\pytest
```

## 本地 AI

分析引擎默认是确定性的。本地模型是可选增强层，只能基于结构化证据润色表达。

OpenAI-compatible 本地 llama.cpp 接口：

```powershell
$env:TABLEPILOT_ENABLE_LOCAL_AI = "1"
$env:TABLEPILOT_LOCAL_AI_PROVIDER = "openai-compatible"
$env:LOCAL_LLM_BASE_URL = "http://127.0.0.1:39281/v1"
$env:LOCAL_LLM_MODEL = "qwen3-4b"
docker compose up --build
```

模型冒烟测试：

```powershell
Invoke-RestMethod "http://127.0.0.1:39281/v1/models"
```

Ollama 仍然支持：

```powershell
$env:TABLEPILOT_ENABLE_LOCAL_AI = "1"
$env:TABLEPILOT_LOCAL_AI_PROVIDER = "ollama"
$env:OLLAMA_MODEL = "qwen2.5:1.5b"
$env:OLLAMA_URL = "http://127.0.0.1:11434/api/generate"
docker compose up --build
```

如果本地模型没开启、不可用，或者没有通过证据护栏，TablePilot 仍然会正常使用规则分析，并在响应里显示本地 AI 状态。当模型提到结构化画像中不存在的字段时，这段模型文本会被抑制，不会进入用户报告。

## 清洗导出接口

```powershell
curl.exe -F "file=@demo/quality_issues_demo.csv;type=text/csv" `
  "http://127.0.0.1:8000/api/clean-upload?format=csv" `
  -o quality_issues_demo-cleaned.csv
```

使用 `format=xlsx` 可以导出一个包含 `cleaned` 和 `repair_summary` 两个工作表的 Excel 文件。

## 报告和会话接口

```powershell
Invoke-RestMethod `
  -Method Post `
  -Uri http://127.0.0.1:8000/api/report/markdown `
  -ContentType "application/json" `
  -Body '{"filename":"tablepilot_demo_sales.xlsx"}'
```

```powershell
Invoke-RestMethod `
  -Method Post `
  -Uri http://127.0.0.1:8000/api/session/export `
  -ContentType "application/json" `
  -Body '{"filename":"tablepilot_demo_sales.xlsx"}'
```

## Windows 发布包

```powershell
powershell -ExecutionPolicy Bypass -File packaging\build-windows-release.ps1
```

输出：

```text
dist/TablePilot.zip
```

发布包包含 Qt 可执行文件、Docker Compose 文件、分析服务、演示数据、配置文件和发布说明。

## 配置与双语文案

产品级配置：

```text
config/app.json
```

中英文文案：

```text
config/i18n.en.json
config/i18n.zh-CN.json
```

当前桌面端已经支持中英文切换。配置文件用于让后续 UI 文案扩展更容易维护。

## 分支

- `main`：当前现代化 TablePilot 版本。
- `legacy-original-qt`：原始 Qt 统计分析版本，仅作为历史保留。

## 后续路线

- 增加拖拽上传启动页和最近文件。
- 增加每个 Chart Studio 视图的图表图片导出。
- 增加 OpenAI-compatible 和 Ollama 本地模型冒烟测试模式。
- 最终 UI 稳定后，把真实截图加入项目主页。
- 增加 Windows 发布包的 GitHub Release 自动化。

## 免责声明

TablePilot 生成的是学习和作品集展示用途的分析摘要，不构成业务、财务或运营建议。
