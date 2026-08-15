#!/usr/bin/env python
"""Bump the version-pinned Windows download badge in the README files.

The READMEs lead the badge row with a Windows download badge hard-pinned to
a specific release tag, e.g.::

    https://github.com/.../releases/download/v1.1.4/TablePilot-v1.1.4.exe
    .../badge/Download_Windows_TablePilot_v1.1.4-0078D6.svg ...

Tagging a new release requires bumping three occurrences of the version in
two README files. Doing it by hand is error-prone and easy to forget, so the
sync-artifacts workflow calls this script right after a tag is published.

Usage:
    python scripts/bump_readme_version.py <new_tag> [--old-tag <old_tag>]

If --old-tag is omitted, the current version is auto-detected from README.md
(by scanning for `releases/download/vX.Y.Z/`). The script refuses to run if
the two READMEs disagree on the current pinned version, so it never silently
diverges them.

Exit codes:
    0  bump applied (or already at target — no-op)
    2  READMEs disagree on current version
    3  could not detect current version
    4  target tag malformed (expected vX.Y.Z)
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
README_FILES = [REPO_ROOT / "README.md", REPO_ROOT / "README.zh-CN.md"]

# vX.Y.Z (no pre-release suffix here — our tags are plain semver)
TAG_RE = re.compile(r"^v(\d+)\.(\d+)\.(\d+)$")
# matches v1.1.4 inside download URLs and badge labels
VERSION_RE = re.compile(r"v\d+\.\d+\.\d+")


def detect_current(readme: Path) -> str | None:
    """Detect the release tag pinned in the Windows download badge of one README."""
    text = readme.read_text(encoding="utf-8")
    # The download URL is the canonical pin location.
    m = re.search(r"releases/download/(v\d+\.\d+\.\d+)/TablePilot-\1\.exe", text)
    return m.group(1) if m else None


def replace_version(text: str, old: str, new: str) -> str:
    """Replace every v-pinned occurrence related to the download badge.

    We scope replacements to the Windows download badge area (the badge block +
    the Quickstart one-click subsection) rather than a global s/vX.Y.Z/vNew/g,
    which would also hit CHANGELOG section headers, commit hash references, etc.
    Doing a targeted replace on the known substrings keeps the script safe.
    """
    replacements = [
        # download URL in the badge href
        (f"releases/download/{old}/TablePilot-{old}.exe",
         f"releases/download/{new}/TablePilot-{new}.exe"),
        # shields.io badge label: Download_Windows_TablePilot_v1.1.4
        (f"Download_Windows_TablePilot_{old}",
         f"Download_Windows_TablePilot_{new}"),
        # zh-CN badge label: 下载_Windows_TablePilot_v1.1.4
        (f"\u4e0b\u8f7d_Windows_TablePilot_{old}",
         f"\u4e0b\u8f7d_Windows_TablePilot_{new}"),
        # alt text mentioning the version (en + zh)
        (f"Download TablePilot for Windows ({old})",
         f"Download TablePilot for Windows ({new})"),
        (f"\u4e0b\u8f7d Windows \u7248 TablePilot\uff08{old}\uff09",
         f"\u4e0b\u8f7d Windows \u7248 TablePilot\uff08{new}\uff09"),
        # Quickstart one-click subsection inline links + prose
        (f"`TablePilot-{old}.exe`",
         f"`TablePilot-{new}.exe`"),
        (f"releases/download/{old}/TablePilot-{old}.exe",
         f"releases/download/{new}/TablePilot-{new}.exe"),
        # any stray "v1.1.4" inside the one-click subsection prose handled below
    ]
    for old_s, new_s in replacements:
        text = text.replace(old_s, new_s)
    # Finally, any remaining bare "v{old}" that lives inside the one-click
    # subsection (e.g. "Tip ... exe ... .") — only touch within that block so
    # we never rewrite CHANGELOG-style headings.
    text = _bump_in_oneclick_block(text, old, new)
    return text


def _bump_in_oneclick_block(text: str, old: str, new: str) -> str:
    """Bump bare v{old} references inside the Quickstart one-click subsection only."""
    # The subsection lives under a `### Windows one-click` / `### Windows 一键安装`
    # heading and ends at the next `### ` or `## ` heading.
    for heading in ("### Windows one-click (prebuilt desktop shell)",
                    "### Windows \u4e00\u952e\u5b89\u88c5\uff08\u9884\u6784\u5efa\u684c\u9762\u58f3\uff09"):
        idx = text.find(heading)
        if idx == -1:
            continue
        end_marker = "\n### "
        end = text.find(end_marker, idx + len(heading))
        if end == -1:
            end_marker = "\n## "
            end = text.find(end_marker, idx + len(heading))
        if end == -1:
            # subsection runs to EOF — rare; treat whole tail as the block
            end = len(text)
        block = text[idx:end]
        block = block.replace(old, new)
        text = text[:idx] + block + text[end:]
    return text


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("new_tag", help="target release tag, e.g. v1.1.5")
    ap.add_argument("--old-tag", dest="old_tag",
                    help="current pinned tag; auto-detected from README.md if omitted")
    args = ap.parse_args()

    if not TAG_RE.match(args.new_tag):
        print(f"ERROR: new tag {args.new_tag!r} does not match vX.Y.Z", file=sys.stderr)
        return 4

    old = args.old_tag or detect_current(README_FILES[0])
    if not old:
        print("ERROR: could not detect current pinned version in README.md "
              "(looking for `releases/download/vX.Y.Z/TablePilot-vX.Y.Z.exe`)",
              file=sys.stderr)
        return 3

    if not TAG_RE.match(old):
        print(f"ERROR: detected current tag {old!r} is not vX.Y.Z", file=sys.stderr)
        return 3

    # Guard: both READMEs must agree on the current pin (they should be kept in
    # sync; refusing here prevents silently diverging them on bump).
    cur_zh = detect_current(README_FILES[1])
    if cur_zh is not None and cur_zh != old:
        print(f"ERROR: READMEs disagree on current pinned version: "
              f"README.md={old}, README.zh-CN.md={cur_zh}. "
              f"Fix the divergence by hand first.", file=sys.stderr)
        return 2

    if old == args.new_tag:
        print(f"Both READMEs already pin {old}; nothing to bump.")
        return 0

    changed = []
    for readme in README_FILES:
        text = readme.read_text(encoding="utf-8")
        new_text = replace_version(text, old, args.new_tag)
        if new_text != text:
            readme.write_text(new_text, encoding="utf-8")
            changed.append(readme.name)

    if not changed:
        print(f"WARNING: no occurrences of {old} replaced in either README. "
              f"The badge may have been edited; inspect manually.", file=sys.stderr)
        return 3

    print(f"Bumped Windows download badge {old} -> {args.new_tag} in: "
          + ", ".join(changed))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
