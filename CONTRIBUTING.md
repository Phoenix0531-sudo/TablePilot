# Contributing to TablePilot

Thanks for considering a contribution. TablePilot is a **local-first messy-table analysis
workbench** — a Python FastAPI service (`analysis_service/`, v0.5.0) plus a Qt/C++ desktop
shell. Keeping it honest and local is the most important constraint; please read this short
guide before opening a PR.

## Ground rules

- **Local-first.** No new feature may upload data to a remote host unless explicitly
  approved in an issue first. The optional `local_ai` path points at a *local* model
  (e.g. `ollama`); do not introduce remote calls into the default code path.
- **Evidence-grounded.** Agent / report output must derive from the actual profiled data,
  not from fabricated claims. See `build_insight_cards` / `build_decision_brief` /
  `answer_question` in `analysis_service/app/` for the established pattern.
- **Don't overstate capabilities.** README, `docs/API.md`, and the project page describe
  what the code actually ships. When you add a capability, update those docs in the same PR.
- **Service is the contract.** Every feature is an HTTP endpoint. Do not add hidden
  cross-process coupling between the desktop shell and the service.

## Setup

```bash
git clone https://github.com/Phoenix0531-sudo/TablePilot.git
cd TablePilot/analysis_service
pip install -r requirements.txt
uvicorn app.main:app --reload
```

The demo dataset directory (`../demo`) is scanned automatically by `GET /api/datasets`.

## Before you open a PR

1. **Reproduce first.** If it's a bug, reproduce it on a `demo/` sample table whenever
   possible and note which table and which endpoint.
2. **Run the tests.**
   ```bash
   python -m pytest -q analysis_service/tests
   bash scripts/smoke.sh   # optional, against a running uvicorn — see scripts/smoke.sh
   ```
3. **Keep changes small.** One concern per PR. Service behavior, desktop shell, docs, and
   CI are separate concerns.
4. **Update docs.** If you touch an endpoint, update `docs/API.md` and the relevant README
   section in the same PR. If you add a screenshot or asset, place it under `assets/` or
   `docs/screenshots/` and reference it locally (no external image links).
5. **Don't hand-edit generated artifacts.** The following files are produced by scripts,
   not authored by hand — regenerate them instead:
   - `docs/openapi.json` — from `scripts/dump_openapi.py`
   - `docs/screenshots/{banner,avatar}{,@2x}.png` — from `scripts/render_assets.py`
   - `docs/screenshots/api-analyze-sample.json` — from `scripts/smoke.sh` against a running uvicorn

   Locally you can run those scripts directly (see their headers). On GitHub, the
   `.github/workflows/sync-artifacts.yml` workflow (`workflow_dispatch`) regenerates any
   subset and opens a PR with the result — trigger it from the Actions tab when docs and
   code drift.
6. **Describe honestly in CHANGELOG.** Add an entry under `[Unreleased]` (or the next
   version) using the Keep a Changelog categories (Added / Changed / Fixed / Removed).

## Areas that especially welcome help

- **Endpoint behavior tests** — `analysis_service/tests/test_analysis_service.py` covers
  happy paths and `test_endpoints_extra.py` covers the error and intent-routing gaps. New
  fixtures and negative-path tests raise confidence further.
- **Sample tables** — small, realistic, anonymized demo tables that exercise specific
  messy-data patterns (encoding fallback, late headers, multi-sheet, whitespace-delimited).
- **Docs** — clearer walkthroughs, more `curl` examples, translation accuracy for
  `README.zh-CN.md`.

## Issue & PR templates

Use the GitHub issue/PR templates in `.github/`. Bug reports ask for the offending
endpoint, the sample table, and how you ran the service; PRs ask which surfaces you
touched and which verification commands you actually ran.

## License

By contributing you agree your contributions are licensed under the project's MIT license
(see [LICENSE](LICENSE)).
