# Lessons

- Do not leave chart areas with empty axes. If a chart has no usable data, render a clear empty state; if data exists, default to an auto-recommended chart.
- Keep user-facing reports separate from developer traces. Parser details, field internals, and guardrail wording belong in diagnostics or muted footnotes, not as the main report.
- Clean export must not re-infer headers after the file has already been normalized; otherwise the first data row can be mistaken for field names.

