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

- 默认不需要 Ollama。
- 如果要启用本地模型表达增强，请先启动 Ollama，并设置 `TABLEPILOT_ENABLE_OLLAMA=1`、`OLLAMA_MODEL=qwen2.5:1.5b`。
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

- Ollama is not required by default.
- To enable local wording enhancement, start Ollama and set `TABLEPILOT_ENABLE_OLLAMA=1` plus `OLLAMA_MODEL=qwen2.5:1.5b`.
- If the local model is unavailable, TablePilot still runs with deterministic analysis results.
