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
