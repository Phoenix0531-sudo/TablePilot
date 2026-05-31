# TablePilot

[![CI](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/Phoenix0531-sudo/TablePilot/actions/workflows/ci.yml)
[![项目主页](https://img.shields.io/badge/project-page-111111)](https://phoenix0531-sudo.github.io/TablePilot/)

[English](README.md) | [中文](README.zh-CN.md)

TablePilot 是一个本地优先的桌面数据分析工作台，可以把 Excel、CSV 和 TXT 表格转化为可解释的数据画像、质量评分、分析规划和可视化摘要。

这个项目把一个早期 Qt 统计分析程序升级成了 C++/Python 混合的作品集项目：桌面端由 Qt/C++ 提供，动态解析、字段识别、质量评分、分析规划、报告生成和接口测试由 Dockerized FastAPI 服务完成。

## 为什么做这个项目

很多小型表格工具只适合固定格式或单一演示文件。TablePilot 面向更真实的表格场景：

- 行列数量不固定
- 支持 Excel、CSV、TXT
- 自动识别文本表格的分隔符
- 自动推断字段结构，而不是写死列名
- 分析前先做数据质量评分
- 用可解释的分析规划替代黑盒输出
- 桌面端支持中英文切换

## 名字是什么意思

`TablePilot` 的意思是“表格数据驾驶员 / 表格分析助手”。它不是替代分析师，而是帮助用户快速加载陌生表格、理解字段结构、发现质量风险，并选择下一步最值得做的分析。

## 项目亮点

- **动态表格解析**：支持 Excel、CSV、TXT。
- **字段结构识别**：numeric、date、category、text、empty、high-cardinality。
- **数据质量评分**：综合缺失值、重复行、异常值、样本量和可分析性。
- **Analysis Planner**：根据字段结构、趋势、相关性和异常自动推荐下一步分析。
- **Qt 桌面端体验**：动态预览表格、图表、画像卡片和双语洞察面板。
- **本地优先架构**：Python 分析服务通过 Docker 在本地运行。
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
      +--> 解析器和字段识别
      +--> 数据质量评分
      +--> 趋势、相关性、异常
      +--> 分析规划器
      +--> Markdown 报告和 Agent 风格回答
```

## 目录结构

```text
Statistical_Analysis/          Qt/C++ 桌面端
analysis_service/              FastAPI 分析服务
analysis_service/tests/        后端测试与测试夹具
config/                        产品配置和中英文文案
demo/                          手动演示 Excel
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
```

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
- 增加用户可选择的图表字段和图表类型。
- 在桌面端增加结构化报告导出。
- 最终 UI 稳定后，把真实截图加入项目主页。
- 增加 Windows 发布包的 GitHub Release 自动化。

## 免责声明

TablePilot 生成的是学习和作品集展示用途的分析摘要，不构成业务、财务或运营建议。
