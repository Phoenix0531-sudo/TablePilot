# LatticeIQ 中文 README

LatticeIQ 是一个本地优先的 AI 表格分析工作台，用 Qt/C++ 提供桌面端体验，用 Dockerized FastAPI 服务完成动态解析、字段识别、数据质量评分、分析规划和可解释输出。

## 名字是什么意思

`Lattice` 是“格架、网格、晶格”的意思，适合表达表格、字段关系和结构化数据网络。`IQ` 表示 intelligence quotient，也就是智能分析能力。  
`LatticeIQ` 的含义是：**把杂乱表格里的结构关系识别出来，并转化为可解释的智能分析。**

## 项目亮点

- 支持 Excel、CSV、TXT 表格文件。
- 自动识别 TXT/CSV 的编码、分隔符和表头。
- 自动识别字段类型：numeric、date、category、text、empty、high-cardinality。
- 数据质量评分：缺失值、重复行、异常值、样本量、字段可分析性。
- Analysis Planner：根据 schema、质量、趋势、相关性和异常，自动规划下一步分析。
- Qt 桌面端支持中英文切换。
- Docker 本地分析服务，避免污染桌面端环境。
- GitHub Actions CI 和 Windows zip 发布包脚本。
- GitHub Pages 项目展示页。

## 目录结构

```text
Statistical_Analysis/          Qt/C++ 桌面端
analysis_service/              FastAPI 分析服务
analysis_service/tests/        后端测试与测试夹具
demo/                          手动演示用 Excel
packaging/                     Windows 发布包脚本
site/                          GitHub Pages 展示页
qss/                           桌面端样式
```

## 快速运行

启动本地分析服务：

```powershell
docker compose up --build
```

打开 Qt 项目：

```text
Statistical_Analysis/Statistical_Analysis.pro
```

推荐 Kit：

```text
Qt 6.11.1 MinGW 64-bit
```

演示 Excel：

```text
demo/latticeiq_demo_sales.xlsx
```

## 发布包

```powershell
powershell -ExecutionPolicy Bypass -File packaging\build-windows-release.ps1
```

输出：

```text
dist/LatticeIQ.zip
```

## 分支

- `main`：当前现代化版本。
- `legacy-original-qt`：原始 Qt 统计分析版本，仅作为历史保留。
