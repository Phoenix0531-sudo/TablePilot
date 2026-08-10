<!-- Thanks for the PR! Keep this short. Delete lines that don't apply. -->

## What & why
<!-- 1–3 sentences: what this changes and why. Link any issue (#NN). -->

## What surfaces this touches
- [ ] analysis_service (Python / FastAPI)
- [ ] desktop shell (Qt / C++)
- [ ] docs / README / project page
- [ ] CI / .github

## Verification
<!-- How you checked this change. Prefer concrete commands/output over claims. -->
- [ ] `python -m pytest -q analysis_service/tests` (if service touched)
- [ ] `bash scripts/smoke.sh` against a running uvicorn (if endpoints touched)
- [ ] README `docs/API.md` updated if an endpoint changed
- [ ] All listed verification commands actually run and pass

## Scope check
- [ ] Keeps data local-first (no new remote uploads unless explicitly approved)
- [ ] Does not overstate capabilities beyond what the code ships
