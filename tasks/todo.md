# TablePilot polish pass

- [x] Redesign the top command bar so it feels like a compact desktop product shell.
- [x] Add a real Chart Studio with auto chart selection, metric/dimension selectors, grouped bars, scatter, heatmap, and box plot.
- [x] Add one-click cleaned-data export for messy tables.
- [x] Add local model UI controls and status visibility.
- [x] Rewrite the analysis panel for end users rather than developers.
- [x] Verify tests, build the Windows app, run the app, commit, and push.

## Review

- Backend tests passed once after the API changes.
- Windows release build passed after fixing one Qt 6 `qsizetype`/`int` mismatch.
- Final verification passed: 29 backend tests, Windows release package build, Docker rebuild, `/health`, `/api/analyze`, `/api/clean-upload`, and local `qwen3-4b` request.
- TablePilot desktop executable was launched from `dist/TablePilot/release/TablePilot.exe`.

# TablePilot workflow pass

- [x] Add recent files with persistent local history.
- [x] Add anomaly review drawer with cell-level navigation.
- [x] Add clean-up before/after comparison.
- [x] Upgrade analysis output with decision brief, priority findings, evidence, limitations, and next actions.
- [x] Sync README, Chinese README, and project page.
- [x] Verify tests, Docker, desktop build, run app, commit, and push.

## Review

- Backend tests passed: 30 passed.
- Windows release build passed and regenerated `dist/TablePilot.zip`.
- Docker Desktop was started, Docker rebuild passed, `/health` passed, `/api/analyze` returned `decision_brief`, and `/api/clean-preview-upload` returned 200.
- TablePilot desktop executable was launched from `dist/TablePilot/release/TablePilot.exe`.

# TablePilot decision intelligence pass

- [x] Add business-role analysis so the app explains drivers, segments, metric mix, and review priorities.
- [x] Make Chinese insight output fully Chinese except dataset field names and product terms.
- [x] Make English insight output fully English.
- [x] Improve the report panel so it reads like a decision brief instead of developer diagnostics.
- [x] Sync README, Chinese README, GitHub Pages, tests, build, run, commit, and push.

## Review

- Backend tests passed: 30 passed.
- Windows release build passed and regenerated `dist/TablePilot.zip`.
- Docker rebuild passed, `/health` passed, `/api/analyze` returned `business_analysis`, and local llama-compatible model returned `local_ai.status=generated`.
- TablePilot desktop executable was launched from `dist/TablePilot/release/TablePilot.exe`.

# TablePilot final presentation pass

- [x] Align Docker Compose project, image, and container names with TablePilot.
- [x] Compact the Chart Studio header so the chart canvas has enough vertical space.
- [x] Add English and Chinese desktop screenshots to README, Chinese README, and GitHub Pages.
- [x] Build and tag a Windows release package.
- [x] Verify tests, Docker, desktop launch, commit, push, and release tag.

## Review

- Backend tests passed: 30 passed.
- Docker rebuild passed; `tablepilot-analysis-service` reached healthy and `/health` returned ok.
- Windows release build passed and regenerated `dist/TablePilot.zip`.
- Desktop screenshots were recaptured in English and Chinese after compacting Chart Studio and fixing language refresh.
- Release commit and `v0.1.0` tag were prepared for GitHub publishing.
