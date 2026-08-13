# Security Policy

## Reporting a vulnerability

TablePilot is a **local-first** tool — by default it processes files on the
operator's own machine and never uploads data to a remote host. Still, the
analysis service (`analysis_service/`) exposes an HTTP surface, and a
vulnerability there could affect any deployment that runs the service on a
shared host.

**Please do not open a public GitHub issue for security reports.** Instead,
use GitHub's private vulnerability reporting:

1. Go to <https://github.com/Phoenix0531-sudo/TablePilot/security/advisories/new>
2. Fill in the affected endpoint or component, a reproduction, and the impact.
3. Mark the advisory private until a fix is coordinated.

You should receive an initial response within **7 days**.

## Scope

In scope:
- The FastAPI analysis service in `analysis_service/app/` — path traversal via
  `filename` / `dataset`, unsafe deserialization of uploaded files, SSRF via
  the optional `local_ai` remote, and any endpoint that writes outside `DATA_DIR`.
- The Docker image / `docker-compose.yml` — container escape, running as root,
  exfiltration via env vars.

Out of scope:
- The Qt / C++ desktop shell reading local user files (it runs as the user
  already; local privilege is assumed).
- `local_ai` calling a **local** model (e.g. `ollama`) — that path is
  operator-chosen and runs on `localhost`; only an attacker controlling the
  `LOCAL_LLM_BASE_URL` env var would be in scope, and that is an operator
  configuration issue, not a TablePilot vulnerability.

## Supported versions

Only the latest release in the `v1.x` line receives security fixes. See
<https://github.com/Phoenix0531-sudo/TablePilot/releases> for the current
latest. The service component version (`v0.5.0` at the time of writing) is
independent of the repo release version and is documented in
`analysis_service/app/main.py`.
