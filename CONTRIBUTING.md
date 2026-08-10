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
5. **Describe honestly in CHANGELOG.** Add an entry under `[Unreleased]` (or the next
   version) using the Keep a Changelog categories (Added / Changed / Fixed / Removed).

## Areas that especially welcome help

- **Endpoint-level tests** — `analysis_service/tests/` currently covers imports and
  structure; per-endpoint behavior tests would raise confidence.
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
