# Changelog

All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](https://semver.org/).

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

No unreleased changes.

## [1.1.4] - 2026-08-14

Patch release landing the Qt desktop CI and wiring the built exe into releases. No service or source changes; service component version is unchanged (`v0.5.0`).

### Added

- **`qt-desktop.yml` workflow — Qt/C++ desktop shell now compiles on a clean runner.** The desktop half (`Statistical_Analysis/Statistical_Analysis.pro`) previously had **zero CI coverage**; `packaging/build-windows-release.ps1` hardcodes the maintainer's local Qt path (`E:\1_Code\QT\6.11.1`) and was never proven on a clean machine, so the README's "Qt 6.5+" claim was an unenforced assertion. The new `build-windows` job installs stock **Qt 6.8 `win64_msvc2022_64`** via `jurplel/install-qt-action@v4`, sets up the MSVC 2022 dev shell via `vswhere → vcvars64.bat` (no third-party action), runs `qmake -spec win32-msvc CONFIG+=release` + `nmake release`, and uploads `TablePilot.exe` (≈1.02 MB) as a 14-day artifact. Proven green at commit `d76f2ef` (run `31788596183`, 2m19s). This makes the desktop shell compile-breakage visible (removed Qt APIs, missing modules, qcustomplot drift) instead of rotting silently.

- **`qt-desktop.yml` bridges its built `TablePilot.exe` into the tag release assets.** A new `release-assets` job — gated to `push: tags: ['v*']`, isolated with its own `permissions: contents: write` so the build job itself stays read-only — downloads the exe artifact, renames it to `TablePilot-<tag>.exe`, creates a draft release for the tag if one does not yet exist, then uploads the versioned exe with `--clobber`. Tagging a release now lands a downloadable Windows desktop exe alongside the FastAPI service tarball.

### Changed

- **`actions/upload-artifact` bumped `v4 → v7`** in `qt-desktop.yml`, matching `ci.yml`. The `v4` action targets Node 20, which GitHub Actions is deprecating on hosted runners (warnings beginning 2025-09-19); `v7` targets Node 24 and removes the deprecation annotation from the Qt build page.

### Investigated (no change)

- **MinGW build path — evaluated and rejected.** Qt 6.8's official MinGW prebuilt entrypoint (`libQt6EntryPoint.a`) still references the MSVCRT-style import symbol `__imp___argc`, which the MSYS2 `mingw64` GCC 15 runtime no longer exports → link failure (`undefined reference to \`__imp___argc'`). The aqt-bundled `tools_mingw` is GCC 8.1 (has the symbol) but trips a `static_assert` in `QtCore/qcomparehelpers.h` against Qt 6.8 headers → compile failure. aqt does not ship a newer MinGW tool (`tools_mingw1310_64` is not a real aqt tool id). MSVC was chosen instead because it matches how Qt ships its own Windows binaries and sidesteps the runtime ABI mismatch entirely. The `.pro` is compiler-agnostic — the only MinGW-specific flag (`-Wa,-mbig-obj`) is guarded by a `win32-g++` scope.

## [1.1.3] - 2026-08-14

Internal docs/CI polish after `1.1.2`. No service or desktop-shell changes; service component version is unchanged (`v0.5.0`).

### Changed

- **`docker.yml` now prints the built image size** after the `docker build` step (`docker images tablepilot-analysis:ci --format ... {{.Size}} {{.VirtualSize}}`), plus an `::notice` annotation. This is a regression-visibility diagnostic for image bloat — it does not gate the build.
- **`docker.yml` concurrency group** — added `concurrency: { group: docker-${{ github.ref }}, cancel-in-progress: true }`, mirroring `ci.yml` / `pages.yml` / `sync-artifacts.yml`, so a rapid push or review no longer queues redundant ~25-minute Docker builds.

### Investigated (no change)

- **Dockerfile multi-stage split — evaluated and rejected.** The CI build reported the single-stage image at **320 MB** (`Size` 320 MB, `VirtualSize` 319.7 MB, ubuntu-24.04 runner). The threshold to reconsider a builder/runtime split is ~500 MB, so the current Dockerfile is justified: `pandas` ships pre-built wheels (no on-image C compilation), so there is no builder-stage toolchain to strip. Do not re-introduce this question unless the size instrumentation crosses the threshold or a wheel starts requiring compilation.
- **`agent_browser` visual verification on Windows — root-caused and deferred to upstream.** `agent_browser` tool calls failed with `could not verify this managed session's live daemon restore policy`. Traced to `pi-agent-browser-native@0.3.0` on Node 26 + Windows: the managed-session policy probe runs `agent-browser --json --namespace "" --session default session info`, and the wrapper's Windows PowerShell quoting mishandles the **empty** namespace, so the CLI reports `Unknown command: default` and the probe exits `unknown`. Proven not a CLI-version issue: reprobed with a **named** namespace (`--namespace default`) → `active: false` / exit 0; empty namespace → exit 1; both 0.33.2 and 0.34.0 fail identically. Documented as a "Known limitation" under the Pages visual-verification SOP in `CONTRIBUTING.md`; no TablePilot code change is warranted — the static asset-200 + SVG `viewBox`/`<title>` check remains the PR gate.

### Removed

- **`docs/index.md`** — orphan stale readme (`FastAPI-0.104+`, `Qt-6.5+` badges) that nothing referenced: the Pages workflow deploys `site/` (not `docs/`), and no workflow or link points at it. Removed to stop a misleading second readme from drifting further from `README.md` / `README.zh-CN.md`.

## [1.1.2] - 2026-08-13

### Added

- **`SECURITY.md`** at repo root — a private-vulnerability-reporting policy (GitHub Security Advisories, not public issues), with a TurboPilot-specific scope statement (FastAPI HTTP surface + Docker image in scope; local Qt shell reading user files and the operator-chosen `local_ai` localhost path out of scope) and a 7-day initial-response target. GitHub surfaces this in the issue chooser and the Security tab.

## [1.1.1] - 2026-08-13

Post-`1.1.0` documentation-only patches caught by a static audit of rendered surfaces (README badges, Pages hero, About snapshot). No service or CI behavior changes.

### Fixed

- **README FastAPI badge** (`README.md`, `README.zh-CN.md`) — the static shield badge still read `FastAPI-0.115` while `analysis_service/requirements.txt` had already moved to `0.141.1` (`1.1.0` Dependabot bump). Synced the badge to `FastAPI-0.141`.
- **Banner PNG-fallback `srcset`** (`README.md`, `README.zh-CN.md`) — the `<picture>` `<source>` listed `banner.png` and `banner@2x.png` with no density descriptors, so browsers parsed both as `1x` (a duplicate-descriptor invalid srcset) and the HiDPI/retina fallback was never selected. Added explicit `1x` / `2x` descriptors.
- **`GITHUB_ABOUT.md`** — outdated snapshot of the repo's About panel (stale long description, 21 topics while GitHub caps at 20). Rewritten as an honest snapshot of the live values (short description, the live 20-topic set) with a header note to keep it in sync.
- **Endpoint count** (`site/index.html`, `CHANGELOG.md` 1.0.0 entry) — claimed "11 endpoints" / "11 live endpoints" while the actual decorated route count in `app/main.py` and in the generated `docs/openapi.json` is **10** (the OpenAPI snapshot guard already asserts `ops == 10`). Corrected in both places to `10 endpoints` / `10 live endpoints`.

## [1.1.0] - 2026-08-12

Internal cleanup, CI tightening, and dependency bumps landed after the public `1.0.0` cut. No breaking service-contract changes; no new user-facing endpoints. Service component version is unchanged (`v0.5.0`) — these are repo-level and CI-level changes.

### Added

- **`.gitattributes`** at repo root — normalizes text files to LF in the object store, pins Windows-only scripts (`*.ps1`/`*.bat`/`*.cmd`) to CRLF, marks image binaries as `binary`, and adds Linguist hints (`.qss` as CSS, `.ui` as XML). Resolves the recurring `LF will be replaced by CRLF` warnings.
- **Ruff lint configuration** in `analysis_service/pyproject.toml` (`[tool.ruff]` + a starter rule set `E, F, I, UP, B, N`) and a new **`Ruff lint (analysis_service)`** job in the CI workflow. Runs on Python 3.11 with `ruff==0.6.9`. The three initially-suppressed rules `I001` / `UP017` / `UP037` were un-ignored in the same window by applying `ruff --fix` to `app/analysis.py`, `app/main.py`, `tests/test_analysis_service.py` (import sorting, `datetime.UTC` alias, de-quoted forward annotation under PEP 563). Ruff now runs fully active with no semantic changes — all 41 tests still pass.
- **`Sync screenshots from assets/`** step in the `Pages` workflow — copies `assets/screenshots/*.png` and the README hero (`docs/screenshots/banner.svg`, `avatar.svg`) into `site/` before the Pages artifact upload, so `site/` no longer carries a second hand-maintained copy of the same bytes.
- **Banner hero on the project page** — `site/index.html` now opens with `<p class="banner"><img src="banner.svg" ...></p>`, reusing the same README hero. New `.banner` CSS in `site/styles.css` (responsive, centered, max-width 960 px).

### Changed

- **`analysis_service/pyproject.toml`** `version` raised `0.4.0 → 0.5.0` to match the runtime `FastAPI(version="0.5.0")` in `app/main.py`. The package metadata is now self-consistent with the running service.
- **GitHub Actions `CI` workflow** — the `Pytest` step no longer appends `|| true` to the `analysis_service/tests` run, so real test regressions now fail CI instead of being silently swallowed.
- **`docs/CHANGELOG.md`** — this section.

### Fixed

- **`analysis_service/tests/test_endpoints_extra.py`** — corrected three `/api/agent/query` intent assertions that had become latent regressions hidden by the previous `|| true`:
  - `data_quality` case question no longer triggers the `anomaly_review` route (the "风险" keyword was being matched first).
  - `correlation_review` and `dataset_overview` cases now assert against the real `profile_dataset` tool trace (`load_table` / `infer_schema`-class steps) instead of the `plan.tools` placeholder names (`calculate_correlations` / `profile_dataset`), which `answer_question` does not surface verbatim in `tools_used`.
- All 41 tests pass under the new dependencies below.

### Dependencies bumped (via Dependabot, squash-merged to `main`)

| Package / Action | From | To |
| --- | --- | --- |
| `fastapi` | `0.115.6` | `0.141.1` |
| `uvicorn[standard]` | `0.34.0` | `0.52.1` |
| `pandas` | `2.2.3` | `3.0.5` |
| `xlrd` | `2.0.1` | `2.0.2` |
| `python-multipart` | `0.0.20` | `0.0.32` |
| `actions/checkout` | `v4` | `v7` |
| `actions/configure-pages` | `v5` | `v6` |
| `actions/upload-pages-artifact` | `v3` | `v5` |

Eight open Dependabot PRs were closed in this window; `main` is current with no outstanding bot PRs.

### Removed

- **`site/screenshots/*.png`** — three duplicate copies of `assets/screenshots/` content (verified identical by SHA-256). Now built in CI by the `Sync screenshots from assets/` step.

## [1.0.0] - 2026-06-08

First public release. **Local-first messy-table analysis workbench** — a Python FastAPI
analysis service plus a Qt/C++ desktop shell, processing Excel / CSV / TXT on disk without
uploading.

### Added

- **FastAPI analysis service (`analysis_service/`, v0.5.0)** with 10 live endpoints:
  - `GET /health` — liveness / container probe
  - `GET /api/datasets` — list local data directory
  - `POST /api/analyze` and `POST /api/analyze-upload` — table profiling by name or upload
  - `POST /api/clean-preview-upload` — show what a clean would change
  - `POST /api/clean-upload` — return cleaned CSV or XLSX
  - `POST /api/report/markdown` and `POST /api/report/html` — explainable reports
  - `POST /api/agent/query` — free-text question over a table (optional `local_ai`)
  - `POST /api/session/export` — snapshot the current analysis session as JSON
- **Table profiling** — `profile_dataset` / `profile_table`: schema- and quality-oriented
  profile per table; optional Excel `sheet`; optional `local_ai` enhancement.
- **Cleaning pipeline** — `build_cleaning_preview` (before/after) and `build_cleaned_table`
  (CSV/XLSX output, with a `repair_summary` sheet on XLSX).
- **Reports** — `build_html_report` / `build_markdown_report` produce copy-pasteable,
  evidence-grounded artifacts.
- **Agent narrative** — `answer_question` over a table; `local_ai: true` opts into an
  optional local-model path (e.g. `ollama`-style). Deterministic by default.
- **Qt / C++ desktop shell** (`packaging/`, `qss/`, `Statistical_Analysis/`) driving the
  same service surface over HTTP / JSON.
- **CI** — three GitHub Actions workflows: `CI` (Python 3.11, `pytest`), `Docker
  analysis-service` (container health probe), `Pages` (deploys `site/`).
- **Sample tables** in `demo/` — `quality_issues_demo.csv`, `tablepilot_demo_sales.xlsx`,
  `multi_sheet_operations.xlsx`, `time_series_demo.txt`.
- **Project page** (`site/`) served via GitHub Pages.
- **Real UI captures** in `assets/screenshots/` — desktop overview, Chinese insights,
  clean-compare.

### Changed

- **README repositioned** to its true identity — local-first messy-table workbench —
  with banner, one-line promise, badge matrix (CI / Docker / Pages / MIT / Python / FastAPI
  / Qt / Local-first), TOC anchors, Overview, endpoint-grounded Features, Quickstart command
  table, a mermaid architecture, a real-screenshot proof wall, Scope In/Out, a 3-item FAQ,
  and an AI-assisted + hand-verified method note. Synchronized English (`README.md`) and
  Chinese (`README.zh-CN.md`).
- **Project page (`site/index.html`)** aligned to the same positioning and endpoint
  vocabulary.
- **Visual system** — pure SVG hero (`docs/screenshots/banner.svg`, 1200×280) and a 256×256
  repository avatar (`docs/screenshots/avatar.svg`), derived from a project-native table-grid
  motif and an analysis pipeline (Profile → Clean → Report).

### Notes

- The service component versions independently from the repo: **service `v0.5.0`** lives
  inside the `v1.0.0` repository release. They are intentionally decoupled — the service is
  the contract surface, the repo ships the whole workbench.
- Files are local-only by default; TablePilot never uploads data unless the operator
  explicitly configures a remote destination.
