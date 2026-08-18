"""Capture static screenshots of the TablePilot project site.

Stages a self-contained copy of ``site/`` (screenshots + banner/avatar SVGs
synced from ``assets/`` and ``docs/screenshots/`` exactly as the Pages workflow
does), serves it over a local HTTP server, drives a headless Chromium via
Playwright, and writes two stable PNGs:

    site/screenshots/pages-desktop.png   (1280 x 800, full page height)
    site/screenshots/pages-mobile.png    (390 x 844, full page height)

Stable filenames are intentional — CI regenerates these on every run, so a
date-stamped name would accumulate forever in git history; a stable name keeps
each commit a single-file diff. The README Proof section references the desktop
PNG as the live Pages-site preview.

This is run from the ``screenshot-site`` GitHub Actions workflow; it is not
meant to run locally (playwright + chromium install is expensive).

Outputs ``METRIC`` lines so a future bench/coverage job could parse them, and
exits non-zero on any capture failure so the workflow gate is meaningful.
"""

from __future__ import annotations

import http.server
import pathlib
import socketserver
import sys
import threading
import time

HERE = pathlib.Path(__file__).resolve().parent
REPO = HERE.parent
SITE = REPO / "site"
SHOTS = SITE / "screenshots"

DESKTOP = (1280, 800)
MOBILE = (390, 844)


def stage_site() -> None:
    """Copy assets/screenshots into site/screenshots and banner/avatar SVGs
    into site/, mirroring .github/workflows/pages.yml exactly so the
    screenshots reflect what real Pages visitors see."""
    import shutil

    SHOTS.mkdir(parents=True, exist_ok=True)
    for src in (REPO / "assets" / "screenshots").glob("*"):
        shutil.copy2(src, SHOTS / src.name)
    for name in ("banner.svg", "avatar.svg"):
        src = REPO / "docs" / "screenshots" / name
        if src.exists():
            shutil.copy2(src, SITE / name)


def serve(directory: pathlib.Path, port: int) -> socketserver.TCPServer:
    handler = lambda *a, **kw: http.server.SimpleHTTPRequestHandler(  # noqa: E731
        *a, directory=str(directory), **kw
    )
    httpd = socketserver.TCPServer(("127.0.0.1", port), handler)
    t = threading.Thread(target=httpd.serve_forever, daemon=True)
    t.start()
    return httpd


def capture(playwright, url: str) -> list[tuple[str, int, int]]:
    browser = playwright.chromium.launch(headless=True)
    results: list[tuple[str, int, int]] = []
    try:
        for label, (w, h) in (("pages-desktop", DESKTOP), ("pages-mobile", MOBILE)):
            ctx = browser.new_context(viewport={"width": w, "height": h},
                                      device_scale_factor=2)
            page = ctx.new_page()
            page.goto(url, wait_until="networkidle", timeout=30_000)
            # Let SVGs + web fonts paint; networkidle catches most, but a
            # deterministic settle lets the hero banner render fully.
            page.wait_for_timeout(800)
            out = SHOTS / f"{label}.png"
            page.screenshot(path=str(out), full_page=True)
            size = out.stat().st_size
            if size < 2_000:
                raise RuntimeError(f"{out} too small ({size}B) — likely blank page")
            print(f"METRIC {label}_bytes={size}")
            results.append((label, w, h))
            ctx.close()
    finally:
        browser.close()
    return results


def main() -> int:
    stage_site()

    from playwright.sync_api import sync_playwright

    port = 8931
    httpd = serve(SITE, port)
    url = f"http://127.0.0.1:{port}/"
    # Brief settle so the listener is bound before we connect.
    time.sleep(0.5)
    try:
        with sync_playwright() as pw:
            captured = capture(pw, url)
    finally:
        httpd.shutdown()

    for label, w, h in captured:
        print(f"captured {label}.png @ {w}x{h}")
    print("METRIC screenshots_captured=" + str(len(captured)))
    if len(captured) != 2:
        print("ERROR: expected 2 screenshots", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
