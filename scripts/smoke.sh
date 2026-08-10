#!/usr/bin/env bash
# TablePilot analysis service — smoke test against a running uvicorn instance.
#
# This script does NOT start the service for you. Start it first:
#
#     cd analysis_service
#     pip install -r requirements.txt
#     uvicorn app.main:app --reload
#
# Then from the repo root:
#
#     bash scripts/smoke.sh
#
# It prints the HTTP status of each call and writes one captured response
# (the /api/analyze profile for quality_issues_demo.csv) to
# docs/screenshots/api-analyze-sample.json so the README proof wall stays
# grounded in real output rather than a fabricated promise.
#
# Env overrides:
#   TP_BASE   service base URL (default http://127.0.0.1:8000)
#   TP_SAMPLE  dataset filename to profile (default quality_issues_demo.csv)
#   TP_OUT     where to write the captured JSON (default docs/screenshots/api-analyze-sample.json)

set -u

TP_BASE="${TP_BASE:-http://127.0.0.1:8000}"
TP_SAMPLE="${TP_SAMPLE:-quality_issues_demo.csv}"
TP_OUT="${TP_OUT:-docs/screenshots/api-analyze-sample.json}"

ok=0
fail=0

probe() {
    local label="$1"; shift
    local url="$1"; shift
    local code
    code="$(curl -s -o /dev/null -w '%{http_code}' "$url" 2>/dev/null || echo 000)"
    if [ "$code" = "200" ]; then
        printf '  [OK]   %-30s %s\n' "$label" "$code"
        ok=$((ok + 1))
    else
        printf '  [FAIL] %-30s %s\n' "$label" "$code"
        fail=$((fail + 1))
    fi
}

echo "TablePilot service smoke test against $TP_BASE"
echo

probe "GET  /health"           "$TP_BASE/health"
probe "GET  /api/datasets"     "$TP_BASE/api/datasets"
probe "POST /api/analyze"      "-X POST $TP_BASE/api/analyze -H 'Content-Type: application/json' -d \"{\\\"filename\\\":\\\"$TP_SAMPLE\\\"}\" >/dev/null" >/dev/null 2>&1
# the probe above is a status-only shorthand; the real capture is below

echo
echo "Capturing POST /api/analyze ($TP_SAMPLE) -> $TP_OUT"
if curl -s -X POST "$TP_BASE/api/analyze" \
        -H 'Content-Type: application/json' \
        -d "{\"filename\":\"$TP_SAMPLE\"}" \
        -o "$TP_OUT" 2>/dev/null; then
    # pretty-print in place if python is available, otherwise leave compact JSON
    if command -v python >/dev/null 2>&1; then
        python -m json.tool "$TP_OUT" > "$TP_OUT.tmp" 2>/dev/null && mv "$TP_OUT.tmp" "$TP_OUT"
    fi
    echo "  [OK]   captured $(wc -c < "$TP_OUT") bytes"
    ok=$((ok + 1))
else
    echo "  [FAIL] could not reach $TP_BASE/api/analyze"
    fail=$((fail + 1))
fi

echo
echo "Result: $ok ok, $fail fail"
[ "$fail" -eq 0 ]
