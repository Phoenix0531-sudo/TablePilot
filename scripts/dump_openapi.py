#!/usr/bin/env python3
"""Dump the FastAPI OpenAPI schema for analysis_service to docs/openapi.json.

This is a reproducible artifact generator, not a runtime dependency: the
analysis service exposes /openapi.json live at <base>/openapi.json, but
committing a frozen snapshot lets docs/API.md and external consumers cite a
stable, reviewable schema alongside each release.

No new dependencies: FastAPI builds the OpenAPI document in memory via
app.openapi(). Run from the repo root:

    python scripts/dump_openapi.py

Env overrides:
    TP_OUT   output path (default docs/openapi.json)
"""

from __future__ import annotations

import json
import os
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
SERVICE_DIR = REPO_ROOT / "analysis_service"
DEFAULT_OUT = REPO_ROOT / "docs" / "openapi.json"


def main() -> int:
    # app/ uses absolute imports (`from .analysis import ...`), so import as a
    # package: add analysis_service to sys.path and import app.main.
    sys.path.insert(0, str(SERVICE_DIR))
    from app.main import app  # noqa: E402

    schema = app.openapi()
    out = Path(os.environ.get("TP_OUT", DEFAULT_OUT))
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(schema, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"wrote {out} ({out.stat().st_size} bytes)", flush=True)
    print(f"openapi: {schema.get('openapi')}", flush=True)
    print(f"info.title: {schema.get('info', {}).get('title')}", flush=True)
    print(f"info.version: {schema.get('info', {}).get('version')}", flush=True)
    paths = list(schema.get("paths", {}).keys())
    print(f"paths ({len(paths)}): {', '.join(paths)}", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
