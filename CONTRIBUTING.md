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

## Visual verification of the project page

The GitHub Pages site (`site/index.html` deployed at
`https://phoenix0531-sudo.github.io/TablePilot/`) is the public landing surface — you
should sanity-check it renders after any `site/` or hero-asset change.

### Quick static check (no dependencies)

```bash
# Every referenced asset returns 200 and banner.svg is a real SVG with viewBox + <title>:
for u in / \      styles.css banner.svg avatar.svg \
      screenshots/tablepilot-desktop-overview.png \
      screenshots/tablepilot-chinese-insights.png \
      screenshots/tablepilot-clean-compare.png ; do
      code=$(curl -s -o /dev/null -w '%{http_code}' "https://phoenix0531-sudo.github.io/TablePilot$u")
      printf '%s\t%s\n' "$code" "$u"
    done
curl -s https://phoenix0531-sudo.github.io/TablePilot/banner.svg | grep -q viewBox \
  && echo 'banner viewBox: ok'
```

### Browser screenshot (optional, needs upstream `agent-browser` CLI)

For a real rendered screenshot under Pi's `agent_browser` tool, install the upstream
CLI once on your machine (`npm i -g agent-browser`; see
<https://agent-browser.dev/>), then:

```
# inside Pi:
> Use the agent_browser tool to open https://phoenix0531-sudo.github.io/TablePilot/
    and take a screenshot to /tmp/pages-banner.png
```

This step is **not** required for a PR — the static check above is enough — but it's
the fastest way to verify that a new `<picture>` / srcset or banner edit actually
renders in a browser.

#### Known limitation: `agent_browser` on Windows + empty session namespace

If invoking `agent_browser` fails with
`The wrapper could not verify this managed session's live daemon restore policy`,
it is an upstream `pi-agent-browser-native` Windows defect, **not** a TablePilot
issue. Under the hood the managed-session policy probe runs
`agent-browser --json --namespace "" --session default session info`; the wrapper's
Windows PowerShell wrapper mishandles the empty-namespace quote, so the CLI reports
`Unknown command: default` and the probe returns `unknown`. Repro and workaround were
confirmed against `pi-agent-browser-native@0.3.0` on Node 26 + Windows: probing with
a **named** namespace (`--namespace default`) returns `active: false` / exit 0 and
succeeds, while the empty-namespace probe exits 1. Until the extension fixes the
empty-namespace quoting on Windows, fall back to the static check above or verify in
a normal browser window. CLI version (0.33.2 vs 0.34.0) is **not** the cause — both
fail the same way here.

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
