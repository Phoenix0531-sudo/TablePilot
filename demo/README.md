# Sample tables (`demo/`)

Four small, **fully synthetic** sample tables ship in this directory. They are
the fastest way to exercise every endpoint of the analysis service and the
end-to-end demo script (`scripts/demo_e2e.sh`).

> **No real data.** Every row in every file below was authored by the TablePilot
> maintainer as fabricated sample data for demos, tests, and documentation. The
> values (names, regions, dates, metrics) are invented and do not correspond to
> any real person, organization, or dataset. They contain intentional quality
> issues (duplicates, missing values, anomalies) by design — that is the point of
> a dirty-table analysis workbench.

## Files

| File                          | Type | Rows | Best for                                                      |
| ----------------------------- | ---- | ---- | ------------------------------------------------------------ |
| `quality_issues_demo.csv`     | CSV  | ~14  | First run. Deliberate duplicates, missing cells, and z-score anomalies. Pairs well with `/api/clean-preview-upload` and `/api/clean-upload`. |
| `tablepilot_demo_sales.xlsx`  | XLSX | ~20  | Segment / trend / correlation review. Great target for `/api/agent/query`. |
| `multi_sheet_operations.xlsx` | XLSX | —    | Excel sheet selection. Pass `sheet` to `/api/analyze` or `/api/clean-upload?format=xlsx`. |
| `time_series_demo.txt`        | TXT  | ~12  | Encoding/delimiter autodetection (tab-comma-whitespace fallback). Exercises the text-parsing branch of `load_table_with_metadata`. |

## License

These sample tables are part of the TablePilot repository and are covered by the
same **MIT License** as the source code (see the root [`LICENSE`](../LICENSE)).
Because they are synthetic and contain no personal or proprietary data, they are
safe to redistribute, copy, and modify under the MIT terms.

## Regenerating

The files are committed source, not generated artifacts. To tweak a sample,
edit the file directly and commit — there is no build step. The row counts
above are approximate and may drift as the demos evolve; run
`python -c "import pandas as pd; print(len(pd.read_csv('quality_issues_demo.csv')))"` \
to confirm the exact current size if a test asserts a specific count.
