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
