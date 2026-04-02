import argparse
import json
import re
import subprocess
import unicodedata
from pathlib import Path

REPO = "AvPa1212/PhysEngine"
GH = r"C:\\Program Files\\GitHub CLI\\gh.exe"
SCRIPT_PATH = Path("create_issues.sh")


def run_gh(args: list[str]) -> str:
    return subprocess.check_output([GH, *args], text=True)


def normalize_text(s: str) -> str:
    return (
        s.replace("\ufffd", "-")
        .replace("\u2014", "-")
        .replace("\u2013", "-")
        .replace("\u00d7", "x")
        .replace("\u03a3", "Sigma")
        .strip()
    )


def key_for_title(s: str) -> str:
    base = unicodedata.normalize("NFKC", normalize_text(s))
    base = re.sub(r"\s+", " ", base).strip().casefold()
    return base


def unescape(s: str) -> str:
    return s.replace('\\"', '"').replace('\\`', '`').replace('\\\\', '\\')


def parse_issues(script_text: str) -> list[tuple[str, str, str]]:
    pattern = re.compile(
        r'create\s+\\\s*\n"((?:[^"\\]|\\.)*)"\s+\\\s*\n"((?:.|\n|\r)*?)"\s+\\\s*\n"((?:[^"\\]|\\.)*)"',
        re.MULTILINE,
    )
    matches = list(pattern.finditer(script_text))
    if not matches:
        raise SystemExit("No create entries parsed from create_issues.sh")

    parsed: list[tuple[str, str, str]] = []
    for m in matches:
        title = normalize_text(unescape(m.group(1)))
        body = unescape(m.group(2)).strip()
        label = normalize_text(unescape(m.group(3))).lower() or "enhancement"
        parsed.append((title, body, label))
    return parsed


def main() -> None:
    parser = argparse.ArgumentParser(description="Create missing GitHub issues from create_issues.sh")
    parser.add_argument("--dry-run", action="store_true", help="Only report what would be created")
    args = parser.parse_args()

    text = SCRIPT_PATH.read_text(encoding="utf-8", errors="replace")
    issues = parse_issues(text)

    labels_json = run_gh(["label", "list", "--repo", REPO, "--limit", "200", "--json", "name"])
    available_labels = {item["name"].strip().lower() for item in json.loads(labels_json)}

    existing_json = run_gh(["issue", "list", "--repo", REPO, "--state", "all", "--limit", "500", "--json", "title"])
    existing_titles = {item["title"] for item in json.loads(existing_json)}
    existing_keys = {key_for_title(t) for t in existing_titles}

    created: list[tuple[str, str]] = []
    skipped: list[str] = []
    failed: list[tuple[str, str]] = []

    for title, body, label in issues:
        title_key = key_for_title(title)
        if title_key in existing_keys:
            skipped.append(title)
            continue

        safe_label = label if label in available_labels else "enhancement"
        if args.dry_run:
            print(f"DRYRUN: {title} [{safe_label}]")
            continue

        proc = subprocess.run(
            [
                GH,
                "issue",
                "create",
                "--repo",
                REPO,
                "--title",
                title,
                "--body",
                body,
                "--label",
                safe_label,
            ],
            text=True,
            capture_output=True,
        )

        if proc.returncode != 0:
            failed.append((title, proc.stderr.strip()))
            continue

        url = proc.stdout.strip().splitlines()[-1] if proc.stdout.strip() else "(created)"
        created.append((title, url))
        existing_keys.add(title_key)

    print(f"PARSED={len(issues)}")
    print(f"CREATED={len(created)}")
    print(f"SKIPPED={len(skipped)}")
    print(f"FAILED={len(failed)}")

    for title, url in created:
        print(f"+ {url} :: {title}")
    for title, err in failed:
        print(f"! {title} :: {err}")


if __name__ == "__main__":
    main()
