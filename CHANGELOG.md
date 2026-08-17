# Changelog

All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](https://semver.org/).

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Fixed

- **Removed the dead `Go build (if present)` CI job.** `ci.yml` ran a `go-build` job that set up Go 1.22, scanned for `go.mod` files, and built any Go modules it found. The repository contains **zero** `.go` files and no `go.mod` — the job's only output was ever `echo "No go.mod"`, yet it still cost a runner slot, a `setup-go@v7` download, and CI wall-clock time on every push. Removed entirely. No test coverage is lost because the job never tested anything.

- **`analysis_service/pyproject.toml` no longer drifts from `requirements.txt` (single source of truth).** The `[project].dependencies` static list had fallen behind `analysis_service/requirements.txt` — pyproject pinned `fastapi==0.115.6`, `uvicorn==0.34.0`, `pandas==2.2.3`, `xlrd==2.0.1`, `python-multipart==0.0.20` while the live, CI/Docker-exercised `requirements.txt` pinned `0.141.1`, `0.52.2`, `3.0.5`, `2.0.2`, `0.0.32` (openpyxl already matched at `3.1.5`). Rather than just re-sync the static list (which would re-drift on the next Dependabot bump), the package metadata now reads dependencies dynamically via `[tool.setuptools.dynamic] dependencies = { file = ["requirements.txt"] }` with `[project] dynamic = ["dependencies"]` — `requirements.txt` is now the only pinned runtime-dep declaration, so the two can never disagree again.

### Added

- **CI guard for pyproject dynamic-dependency resolution (`package-metadata` job).** Since nothing in CI actually does `pip install analysis_service/` today, the dynamic-dependency wiring was previously un-exercised. A new `package-metadata` job builds the wheel with `python -m build --no-isolation`, parses the resulting `METADATA`, and asserts all 6 expected `Requires-Dist` entries (`fastapi`, `uvicorn`, `pandas`, `openpyxl`, `xlrd`, `python-multipart`) are present — failing loudly if setuptools can no longer read `requirements.txt`.

## [1.1.5] - 2026-08-16

### Added

- **README now ships a runnable end-to-end demo (`scripts/demo_e2e.sh`).** The Features section lists every endpoint, but a reader still could not see *what order to call them in for a real table*. The new script walks one bundled table — `demo/quality_issues_demo.csv` (duplicate rows, missing cells, a revenue outlier) — through the three core endpoint families in 60 seconds: `GET /health` → `GET /api/datasets` → `POST /api/analyze` (prints rows/cols/missing, the 0–100 quality score, anomaly count) → `POST /api/clean-preview-upload` (prints duplicate rows removed, missing cells filled, anomaly rows marked — no file mutation) → `POST /api/report/markdown` (first 20 lines). Both READMEs gained an "End-to-end in 60 seconds" / "60 秒端到端体验" subsection right after the Quickstart command table. All JSON field paths in the script were verified against the real response shapes (`profile.quality.score`, `profile.anomalies`, `clean-preview.summary.{removed_duplicate_rows,filled_missing_cells,marked_anomaly_rows,repairs}`) instead of guessed. A new `demo-e2e` CI job starts the service in the background, runs the script, and greps the output for the three real analysis signals so the demo cannot rot silently.

- **README download badge for the Windows release exe.** Both `README.md` and `README.zh-CN.md` now lead the badge row with a Windows-branded download badge linking directly to `releases/download/v1.1.4/TablePilot-v1.1.4.exe` (the CI-built MSVC artifact pinned in the v1.1.4 release), so visitors land on a one-click Windows install instead of having to drill into the Releases tab. The badge is version-pinned to `v1.1.4` and will need a one-line bump on the next tagged release.

- **README Quickstart now opens with a Windows one-click install subsection.** Both READMEs gained a `Windows one-click (prebuilt desktop shell)` / `Windows 一键安装（预构建桌面壳）` block right under the Quickstart heading: download `TablePilot-v1.1.4.exe` from Releases (~1.0 MB), double-click to launch, then point the shell at a running analysis service (`docker compose` or `uvicorn`) for the `/api/*` features. This closes the badge's conversion path — a visitor who clicks the badge lands on an exe they can immediately run, with the same binary the `qt-desktop` CI produces from `Statistical_Analysis.pro`.

- **`sync-artifacts` workflow can now bump the README badge on a new tag.** A new `artifact_set: readme_badge` option (plus a `release_tag` input, e.g. `v1.1.5`) runs `scripts/bump_readme_version.py`, which rewrites every version-pinned occurrence in the badge + Quickstart one-click subsection of both READMEs from the current tag to the target and opens a PR. The script auto-detects the current pin, refuses to run if the two READMEs disagree, and is a clean no-op when `release_tag` equals the current pin (proven via a `readme_badge`/`v1.1.4` dispatch). This removes the manual one-line bump noted in the badge entry above — tagging the next release + dispatching this set keeps the READMEs in sync without hand-editing.

- **`ci.yml` `openapi-export` job now enforces an endpoint coverage gate.** A new `Endpoint coverage threshold (routes == openapi)` step greps `@app.(get|post|put|delete|patch)(` decorators in `analysis_service/app/main.py` and asserts that count equals both the path count and the operation count in the freshly-dumped `docs/openapi.json`. The existing `Validate schema shape` step already curated a hardcoded expected-path list; the new gate is dynamic — if someone adds/removes a route in `app/main.py` but forgets to regenerate `docs/openapi.json` (and the curated expected list), the count mismatch fails the build instead of letting the contract drift. Currently 10 routes == 10 paths == 10 ops.

- **Qt desktop shell gained a self-contained Web Research dock (Exa search).** New `Statistical_Analysis/webresearchdock.{h,cpp}` adds a `QDockWidget` with a query box + results list that calls the Exa Search API (`POST https://api.exa.ai/search`) via `QNetworkAccessManager`. The `EXA_API_KEY` is read live from the environment at search time (never stored in source); if it is unset the dock shows a friendly "set EXA_API_KEY" hint and fires no network request, so the shell remains usable unconfigured. Results open in the system browser on double-click (also a right-click menu: open / copy URL). A toolbar button toggles the dock; the label and dock strings retranslate with the EN/中文 language switch. The `.pro` adds `webresearchdock.{h,cpp}` to `SOURCES`/`HEADERS` (`network` module was already linked). Verified by the `qt-desktop` CI build producing `TablePilot.exe` on a clean MSVC runner.

- **Real analysis micro-benchmark with CI-gated honesty (`scripts/bench.py` + `bench` CI job).** The README previously had no performance/throughput numbers, and claiming any without measurement would be fabrication. New `scripts/bench.py` times the three pipeline stages (profile → clean-preview → markdown report) on the real bundled `demo/quality_issues_demo.csv` (14 rows) plus profile on a synthetic ~10,000-row re-sampling of the same pattern (explicitly labelled as generated, never claimed as user data) using `time.perf_counter` and the median of 5 runs, in-process (no server/network jitter). It prints `METRIC name=value` lines; a new `bench` CI job runs it and asserts each metric is a positive real number (guards against silent zeros / never-ran paths), so any README number we publish is backed by a CI measurement. Numbers are written into the README only after being observed in a real CI run.

- **README now publishes real, CI-measured performance numbers (gap #4 closed).** Both READMEs gained a `Performance` / `性能` section with a 4-row table: on the real 14-row `demo/quality_issues_demo.csv` — profile ~17 ms, clean-preview ~12 ms, markdown report ~16 ms (median of 5, in-process, on `ubuntu-latest`); and on a synthetic 9,996-row re-sampling of the demo pattern — profile ~346 ms. The section links the live CI job log (run 31948192866) as the source of the numbers, explicitly labels the 10k rows as generated re-sampling (not user data) so it shows only the scaling curve, and notes the `bench` job re-measures on every push and asserts positive numbers so the figures cannot rot. The top nav gained a `Performance` / `性能` anchor. This replaces the prior honest non-claim ("仓厧有 PyTest 但没有 benchmark") with measured data.

### Fixed

- **README banner now renders the SVG instead of a raster PNG that always won `<picture>` source selection (AI review).** The `<picture>` had a PNG `<source srcset="banner.png 1x, banner@2x.png 2x" type="image/png">` with no `media` condition, so it matched in every browser and the vector `banner.svg` `<img>` fallback never rendered — the hero was always the raster PNG, defeating the point of an SVG banner. Both READMEs now keep only the SVG `<img>`, so GitHub renders crisp vectors at any DPI. This also clears a historical display artifact where an earlier `@2x` filename was rendered as a redacted-email placeholder on-screen.

- **README FastAPI badge now shows the exact pinned version `0.141.1` (AI review).** The shields.io label was truncated to `FastAPI-0.141`, masking the real `fastapi==0.141.1` pin in `analysis_service/requirements.txt`. Both READMEs updated to `FastAPI-0.141.1`.

- **README Quickstart `[Releases]` link no longer fires a direct exe download (AI review).** The clickable `[Releases]` text was wired to the raw `releases/download/v1.1.4/TablePilot-v1.1.4.exe` URL, so clicking it downloaded the binary instead of opening the Releases page. Both READMEs now link `[Releases]` to the `/releases` page and make the downloadable filename the exe link. `scripts/bump_readme_version.py` stays compatible — `detect_current`'s `releases/download/vX.Y.Z/TablePilot-vX.Y.Z.exe` regex still matches the exe download link, and the one-click-block bump still covers the prose.

- **README professionalism pass (8 reviewer findings).** Both READMEs tidied: (1) removed the duplicate plain-text title under the banner SVG so the hero isn't two stacked titles; (2) re-indented the misaligned `qt-desktop` badge to match the other five badges' 2-space column; (3) extended the top nav to cover `#proof` and `#contributing` (previously only Overview/Features/Quickstart/Architecture/Scope/FAQ were linked); (4) added a `python -m venv .venv` hint to the from-source Quickstart so users don't install into system Python; (5) fixed the Quickstart table's "Build the desktop shell" row which pointed at `packaging/` and contradicted the Architecture section — it now names `Statistical_Analysis/Statistical_Analysis.pro` + Qt 6 + MSVC (with `packaging/` as the secondary path); (6) annotated the bare `tests/` line in the repo layout with `# pytest suite (analysis_service + standalone smoke)` to match the per-line comment pattern; (7) added a "See also: CHANGELOG · SECURITY · API" link row under Contributing; (8) updated `packaging/`'s own layout comment to `desktop packaging / build scripts`.

- **README Proof screenshots now carry visible captions; Architecture diagram now evidence-anchored (professionalism pass 2).** Two honesty gaps closed. (a) The Proof section's four `<table>`-cell images only exposed meaning through `alt=` text (screen readers / hover-tooltips only); a scrolling reader saw "real UI captures" but no per-shot explanation. Both READMEs now add a `<sub>` caption row under each image naming what it demonstrates (desktop overview / Chinese insight cards / dirty-vs-clean side-by-side / architecture schematic), matching the `<figcaption>` already present in `site/index.html`. (b) The mermaid Architecture diagram labeled nodes with real function/endpoint names but never pointed back to the contract artifact; both READMEs now add a sentence stating each node maps to a real route in `docs/API.md` + `docs/openapi.json` (10 paths) and that the `demo-e2e` CI job exercises profile → clean-preview → report end-to-end against a live service — turning the diagram from "self-praise" into "verifiable claim".

- **Lint coverage extended to repo-root `scripts/` and `tests/` (honesty widening).** Previously `ruff` only ran scoped to `analysis_service/` (its own `[tool.ruff]` in `analysis_service/pyproject.toml`, run by a CI job whose `working-directory` is `analysis_service`). The standalone `scripts/` (`bench.py`, `bump_readme_version.py`, `dump_openapi.py`, `render_assets.py`) and the top-level `tests/` suite lived outside that scope and were never linted — a real gap, since those scripts are the ones reviewers and Dependabot-touchable automation run. New `ruff.toml` at the repo root mirrors the `analysis_service` rule set (conservative starter: E/F/I/UP/B/N, same ignores) but is scoped to `scripts/` and `tests/` and excludes `analysis_service/` (already covered), the Qt/C++ `Statistical_Analysis/` shell, and static `site/` content. A new `ruff-lint-root` CI job runs `ruff check scripts tests` on every push.

### Changed

- **Dependency bumps merged via Dependabot (statically risk-assessed, not blind).** `uvicorn[standard]` 0.52.1 → 0.52.2 (patch: fixes bodyless-request receives and improves HTTP/1 request parsing performance) and `actions/download-artifact` v7 → v8 in `qt-desktop.yml`. The `download-artifact` v8 bump is a *major* with two breaking changes — (1) hash-mismatch now errors by default instead of warning; (2) the action no longer auto-unzips non-zipped downloads — but both were statically verified against the repo's actual usage (standard zipped upload artifact + `find`-locate, no `digest-mismatch` override, no raw-file downloads), so they are forward-compatible. No source code changed; the `release-assets` job's behavior is unchanged.

### Fixed

- **Removed two unused imports in `tests/` that the new root lint surfaced (F401).** `import importlib` in `tests/test_smoke.py` and `import importlib.util` in `tests/test_analysis_import.py` were never referenced. They were invisible before because no lint covered `tests/`; the first run of `ruff-lint-root` flagged them and they were removed. This is the诚实 payoff of widening lint coverage — a dead import the eyeball missed.


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
