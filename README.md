# InsightQt AI Workbench

> English version follows the Chinese version.

## 中文说明

InsightQt AI Workbench 是一个从旧版 Qt/C++ 统计分析工具演进而来的本地智能数据分析工作台。原项目可以读取 Excel/TXT 销售数据、计算基础统计量、绘制折线图和柱状图，并导出图表。第一轮现代化保留了 Qt 桌面端，同时新增了 Docker 化的 Python 分析服务，为后续机器学习、Agent 编排和自然语言分析报告打基础。

原始 Qt 版本保留在：

```text
legacy-qt-statistical-analysis
```

现代化开发分支：

```text
modern-ai-analysis-workbench
```

### 当前能力

- Qt/C++ 桌面端：保留原有数据表格、统计、图表和导出功能。
- Qt/C++ 桌面端智能分析：可以把用户选择的 Excel/TXT/CSV 上传到本地分析服务。
- Python 分析服务：读取或接收 Excel、TXT、CSV 数据并返回结构化数据画像。
- 数据画像：行列数量、数值列数量、缺失值、每列均值/标准差/最小值/中位数/最大值。
- 智能分析：数据质量评分、趋势检测、相关性分析、z-score 异常复核。
- Agent 接口：根据问题意图选择 overview、data quality、trend、correlation、anomaly 等分析路径。
- Docker 运行：分析服务可以通过 `docker compose` 一键启动。

### 架构方向

```mermaid
flowchart LR
    A["Qt/C++ Desktop Client"] --> B["Local FastAPI Analysis Service"]
    B --> C["Data Loader"]
    B --> D["Statistical Profiler"]
    B --> E["Anomaly Detection"]
    B --> F["Deterministic Agent Layer"]
```

### 运行 Python 分析服务

使用 Docker：

```bash
docker compose up --build
```

访问：

```text
http://127.0.0.1:8000/health
http://127.0.0.1:8000/api/datasets
```

分析示例数据：

```bash
curl -X POST http://127.0.0.1:8000/api/analyze ^
  -H "Content-Type: application/json" ^
  -d "{\"filename\":\"销售数据.txt\"}"
```

上传任意本地数据文件分析：

```bash
curl -X POST http://127.0.0.1:8000/api/analyze-upload ^
  -F "file=@Statistical_Analysis/销售数据.txt"
```

Agent 风格查询：

```bash
curl -X POST http://127.0.0.1:8000/api/agent/query ^
  -H "Content-Type: application/json" ^
  -d "{\"filename\":\"销售数据.txt\",\"question\":\"这份数据有没有异常？\"}"
```

本地 Python 运行：

```bash
cd analysis_service
python -m pip install -r requirements.txt
uvicorn app.main:app --reload
```

测试：

```bash
cd analysis_service
python -m pytest -q
```

### Qt 桌面端

项目入口：

```text
Statistical_Analysis/Statistical_Analysis.pro
```

推荐 Kit：

```text
Qt 6.11.1 MinGW 64-bit
```

第一轮现代化已经把 `.pro` 中的旧绝对路径改为相对路径，并为 MinGW 添加了 QCustomPlot 兼容参数。旧版 Excel 解析库 `QXsl/lib/libQXlsx.a` 是 32 位静态库，不能直接用于当前 64 位 Qt 6 环境。后续路线是让 Qt 端调用本地 Python 分析服务，逐步移除 Qt 端对旧 QXlsx 的主路径依赖。

第二轮现代化已经移除了 Qt 端对旧 QXlsx 静态库的编译依赖。Excel/TXT/CSV 智能分析由本地 FastAPI 服务承担，Qt 工具栏中的“智能分析”会将用户选择的数据文件上传到 `http://127.0.0.1:8000/api/analyze-upload` 并展示摘要。

### 后续计划

- 增加更完整的右侧 AI Insight 面板，替代当前弹窗摘要。
- 增加本地 Ollama / OpenAI 的增强解释层。
- 增加中英文 UI 和更专业的分析工作台界面。
- 增加打包发布流程。

## English

InsightQt AI Workbench is a modernization of an older Qt/C++ statistical analysis desktop application. The original application reads Excel/TXT sales data, computes basic statistics, renders line/bar charts, and exports charts. This first modernization pass keeps the Qt desktop client while adding a Dockerized Python analysis service as the foundation for future machine learning, Agent orchestration, and natural-language reporting.

The original Qt version is preserved in:

```text
legacy-qt-statistical-analysis
```

Modernization branch:

```text
modern-ai-analysis-workbench
```

### Current Capabilities

- Qt/C++ desktop client: keeps the original table, statistics, charting, and export workflow.
- Qt/C++ smart analysis: uploads selected Excel/TXT/CSV files to the local analysis service.
- Python analysis service: loads or receives Excel, TXT, and CSV datasets and returns structured profiling results.
- Data profiling: rows, columns, numeric columns, missing cells, mean, standard deviation, min, median, and max.
- Smart analysis: data quality score, trend detection, correlation analysis, and z-score anomaly review.
- Agent endpoint: routes questions to overview, data quality, trend, correlation, or anomaly workflows.
- Docker runtime: the analysis service can be started with `docker compose`.

### Architecture Direction

```mermaid
flowchart LR
    A["Qt/C++ Desktop Client"] --> B["Local FastAPI Analysis Service"]
    B --> C["Data Loader"]
    B --> D["Statistical Profiler"]
    B --> E["Anomaly Detection"]
    B --> F["Deterministic Agent Layer"]
```

### Run the Python Analysis Service

With Docker:

```bash
docker compose up --build
```

Open:

```text
http://127.0.0.1:8000/health
http://127.0.0.1:8000/api/datasets
```

Analyze a sample dataset:

```bash
curl -X POST http://127.0.0.1:8000/api/analyze ^
  -H "Content-Type: application/json" ^
  -d "{\"filename\":\"销售数据.txt\"}"
```

Analyze an uploaded local file:

```bash
curl -X POST http://127.0.0.1:8000/api/analyze-upload ^
  -F "file=@Statistical_Analysis/销售数据.txt"
```

Agent-style query:

```bash
curl -X POST http://127.0.0.1:8000/api/agent/query ^
  -H "Content-Type: application/json" ^
  -d "{\"filename\":\"销售数据.txt\",\"question\":\"这份数据有没有异常？\"}"
```

Run locally with Python:

```bash
cd analysis_service
python -m pip install -r requirements.txt
uvicorn app.main:app --reload
```

Tests:

```bash
cd analysis_service
python -m pytest -q
```

### Qt Desktop Client

Project entry:

```text
Statistical_Analysis/Statistical_Analysis.pro
```

Recommended Kit:

```text
Qt 6.11.1 MinGW 64-bit
```

This pass changes the old absolute paths in the `.pro` file to relative project paths and adds a MinGW compatibility flag for QCustomPlot. The legacy Excel library `QXsl/lib/libQXlsx.a` is a 32-bit static library and cannot be linked directly into the current 64-bit Qt 6 environment. The modernization path is to let the Qt client call the local Python analysis service and gradually remove the Qt-side dependency on the old QXlsx main path.

The second modernization pass removes the Qt-side build dependency on the legacy QXlsx static library. Excel/TXT/CSV smart analysis is handled by the local FastAPI service. The Qt toolbar action `智能分析` uploads the selected file to `http://127.0.0.1:8000/api/analyze-upload` and displays an evidence-based summary.
