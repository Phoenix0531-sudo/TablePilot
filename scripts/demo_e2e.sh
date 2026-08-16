#!/usr/bin/env bash
# End-to-end TablePilot demo: profile → clean-preview → report.
#
# Runs against a local analysis service (start it first with
# `uvicorn app.main:app --reload` from analysis_service/, or `docker compose
# up --build`). The script walks one real bundled table — demo/
# quality_issues_demo.csv, which has duplicate rows, missing cells, and a
# revenue outlier — through the three endpoint families so you can see what
# TablePilot actually does, not just read the endpoint list.
#
# Usage:
#   bash scripts/demo_e2e.sh                 # http://localhost:8000
#   BASE=http://localhost:9000 bash scripts/demo_e2e.sh
#
# Requires: curl + python3 (for JSON field extraction). No jq dependency.
# Exits non-zero on the first failing step so CI can gate on it.

set -euo pipefail

BASE="${BASE:-http://localhost:8000}"
DEMO_CSV="demo/quality_issues_demo.csv"

say() { printf '\n\033[1;36m== %s ==\033[0m\n' "$*"; }
# $1 is a python expression referencing `d` (the parsed JSON). Pass it via
# an env var so bash never has to quote-splice it into `python3 -c`; this
# sidesteps every f-string / nested-quote / backslash hazard.
field() { EXPR="$1" python3 -c 'import sys,json,os; d=json.load(sys.stdin); print(eval(os.environ["EXPR"]))'; }

say "1/5  health check  (GET /health)"
curl -fsS "$BASE/health" | field "d['status']" | sed 's/^/  status: /'

say "2/5  list datasets  (GET /api/datasets)"
curl -fsS "$BASE/api/datasets" \
  | field "', '.join('{} ({}B)'.format(x['filename'], x['size_bytes']) for x in d['datasets'])" \
  | sed 's/^/  files: /'

say "3/5  profile  (POST /api/analyze  → quality_issues_demo.csv)"
PROFILE_JSON=$(curl -fsS -X POST "$BASE/api/analyze" \
  -H 'Content-Type: application/json' \
  -d '{"filename":"quality_issues_demo.csv"}')
echo "$PROFILE_JSON" | field "'  rows: {}\n  cols: {}\n  missing cells: {}'.format(d['dataset']['rows'], d['dataset']['columns'], d['dataset']['missing_cells'])"
echo "$PROFILE_JSON" | field "'  quality score: {}'.format(d.get('quality', {}).get('score', 'n/a'))"
echo "$PROFILE_JSON" | field "'  anomalies: {}'.format(len(d.get('anomalies', [])))"

say "4/5  clean preview  (POST /api/clean-preview-upload  → upload the same csv)"
curl -fsS -X POST "$BASE/api/clean-preview-upload" \
  -F "file=@${DEMO_CSV}" \
  | field "'  repairs available: {}\n  dup rows removed: {}\n  missing cells filled: {}\n  anomaly rows marked: {}'.format(len(d['summary']['repairs']), d['summary']['removed_duplicate_rows'], d['summary']['filled_missing_cells'], d['summary']['marked_anomaly_rows'])"

say "5/5  report  (POST /api/report/markdown)"
REPORT=$(curl -fsS -X POST "$BASE/api/report/markdown" \
  -H 'Content-Type: application/json' \
  -d '{"filename":"quality_issues_demo.csv"}')
echo "$REPORT" | sed -n '1,20p' | sed 's/^/  /'

say "done — open $BASE/docs in a browser for the full Swagger UI."
