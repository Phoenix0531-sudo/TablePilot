#!/usr/bin/env python3
"""Render docs/screenshots/{banner,avatar}.png from their SVG sources.

GitHub renders inline SVG in most places (repo pages, project pages), but not
in email notifications or RSS readers. A PNG fallback shipped alongside the
SVG lets those surfaces still show the hero. This script depends on cairosvg
(itself a thin wrapper over cairo); CI installs it on demand.

Run from the repo root:

    python scripts/render_assets.py

Env overrides:
    TP_RESOLUTIONS  comma-separated DPI/multipliers (default 1,2)
"""

from __future__ import annotations

import os
import sys
from pathlib import Path

try:
    import cairosvg  # type: ignore
except ModuleNotFoundError:
    print(
        "cairosvg is required. Install it with:  pip install cairosvg",
        file=sys.stderr,
    )
    raise

REPO_ROOT = Path(__file__).resolve().parents[1]
SRC_DIR = REPO_ROOT / "docs" / "screenshots"

TARGETS: list[tuple[str, int, int]] = [
    ("banner", 1200, 280),
    ("avatar", 256, 256),
]


def render(name: str, width: int, height: int) -> list[Path]:
    svg = SRC_DIR / f"{name}.svg"
    if not svg.exists():
        raise FileNotFoundError(f"missing SVG source: {svg}")

    multipliers = [float(x) for x in os.environ.get("TP_RESOLUTIONS", "1,2").split(",") if x.strip()]
    written = []
    for m in multipliers:
        out = SRC_DIR / (f"{name}.png" if m == 1 else f"{name}@{int(m)}x.png")
        cairosvg.svg2png(
            url=str(svg),
            write_to=str(out),
            output_width=int(width * m),
            output_height=int(height * m),
        )
        written.append(out)
        print(f"  wrote {out.relative_to(REPO_ROOT)} ({out.stat().st_size} bytes, {int(width*m)}x{int(height*m)})", flush=True)
    return written


def main() -> int:
    print("Rendering SVG -> PNG fallbacks:", flush=True)
    all_written: list[Path] = []
    for name, w, h in TARGETS:
        all_written.extend(render(name, w, h))
    print(f"done: {len(all_written)} file(s)", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
