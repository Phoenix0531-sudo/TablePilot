# LatticeIQ Release Package

## 中文

这个目录是 Windows 发布包结构示例。

运行前要求：

- 已安装 Docker Desktop。
- Docker Desktop 正在运行。

启动方式：

```powershell
powershell -ExecutionPolicy Bypass -File .\start-latticeiq.ps1
```

脚本会先启动本地 FastAPI 分析服务，然后打开 Qt 桌面端。

## English

This folder is the Windows release package layout.

Requirements:

- Docker Desktop is installed.
- Docker Desktop is running.

Start:

```powershell
powershell -ExecutionPolicy Bypass -File .\start-latticeiq.ps1
```

The script starts the local FastAPI analysis service and then opens the Qt desktop workbench.
