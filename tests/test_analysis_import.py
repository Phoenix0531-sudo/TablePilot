"""Lightweight bridge tests for TablePilot analysis service.

Full demo profiling stays in analysis_service/tests (needs pandas/demo files).
"""

from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ANALYSIS = ROOT / "analysis_service" / "app"


def test_analysis_modules_exist():
    assert (ANALYSIS / "main.py").exists()
    assert (ANALYSIS / "analysis.py").exists()


def test_analysis_py_compiles():
    src = (ANALYSIS / "analysis.py").read_text(encoding="utf-8", errors="replace")
    compile(src, str(ANALYSIS / "analysis.py"), "exec")


def test_demo_assets_present():
    demo = ROOT / "demo"
    if not demo.exists():
        return
    files = list(demo.glob("*.xlsx")) + list(demo.glob("*.csv")) + list(demo.glob("*.txt"))
    assert files, "demo/ exists but has no table fixtures"
