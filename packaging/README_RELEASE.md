# TablePilot Release Package

## 中文

这个目录是 Windows 发布包结构示例。

运行前要求：

- 已安装 Docker Desktop。
- Docker Desktop 正在运行。

启动方式：

```powershell
powershell -ExecutionPolicy Bypass -File .\start-tablepilot.ps1
```

脚本会先启动本地 FastAPI 分析服务，然后打开 Qt 桌面端。

可选本地模型：

- 默认不需要本地模型。
- 如果要启用 OpenAI-compatible llama.cpp 接口，请设置 `TABLEPILOT_ENABLE_LOCAL_AI=1`、`TABLEPILOT_LOCAL_AI_PROVIDER=openai-compatible`、`LOCAL_LLM_BASE_URL=http://127.0.0.1:39281/v1`、`LOCAL_LLM_MODEL=qwen3-4b`。
- 如果要启用 Ollama，请设置 `TABLEPILOT_ENABLE_LOCAL_AI=1`、`TABLEPILOT_LOCAL_AI_PROVIDER=ollama`、`OLLAMA_MODEL=qwen2.5:1.5b`。
- 即使本地模型不可用，TablePilot 也会继续使用确定性分析结果。

## English

This folder is the Windows release package layout.

Requirements:

- Docker Desktop is installed.
- Docker Desktop is running.

Start:

```powershell
powershell -ExecutionPolicy Bypass -File .\start-tablepilot.ps1
```

The script starts the local FastAPI analysis service and then opens the Qt desktop workbench.

Optional local model:

- A local model is not required by default.
- To enable an OpenAI-compatible llama.cpp endpoint, set `TABLEPILOT_ENABLE_LOCAL_AI=1`, `TABLEPILOT_LOCAL_AI_PROVIDER=openai-compatible`, `LOCAL_LLM_BASE_URL=http://127.0.0.1:39281/v1`, and `LOCAL_LLM_MODEL=qwen3-4b`.
- To enable Ollama, set `TABLEPILOT_ENABLE_LOCAL_AI=1`, `TABLEPILOT_LOCAL_AI_PROVIDER=ollama`, and `OLLAMA_MODEL=qwen2.5:1.5b`.
- If the local model is unavailable, TablePilot still runs with deterministic analysis results.
