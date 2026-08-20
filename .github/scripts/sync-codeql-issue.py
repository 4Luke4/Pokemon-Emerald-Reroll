#!/usr/bin/env python3
"""Synchronize one GitHub issue with every open default-branch CodeQL alert."""

from __future__ import annotations

import datetime as dt
import json
import os
import urllib.error
import urllib.parse
import urllib.request
from typing import Any


API_ROOT = "https://api.github.com"
API_VERSION = "2026-03-10"
REPORT_MARKER = "<!-- pokemon-emerald-reroll-codeql-report -->"
MAX_PAGE_SIZE = 100


def required_environment(name: str) -> str:
    value = os.environ.get(name)
    if not value:
        raise RuntimeError(f"required environment variable is missing: {name}")
    return value


def api_request(
    token: str,
    method: str,
    path: str,
    payload: dict[str, Any] | None = None,
) -> Any:
    """Call GitHub's versioned JSON API and surface the response body on failure."""

    data = None if payload is None else json.dumps(payload).encode("utf-8")
    request = urllib.request.Request(
        f"{API_ROOT}{path}",
        data=data,
        method=method,
        headers={
            "Accept": "application/vnd.github+json",
            "Authorization": f"Bearer {token}",
            "X-GitHub-Api-Version": API_VERSION,
            "User-Agent": "pokemon-emerald-reroll-codeql-sync",
        },
    )
    try:
        with urllib.request.urlopen(request) as response:
            return json.load(response)
    except urllib.error.HTTPError as error:
        response_body = error.read().decode("utf-8", errors="replace")
        raise RuntimeError(
            f"GitHub API {method} {path} failed with HTTP {error.code}: {response_body}"
        ) from error


def paginated_get(
    token: str,
    path: str,
    parameters: dict[str, str],
) -> list[dict[str, Any]]:
    """Read every page instead of silently dropping findings after the first 100."""

    results: list[dict[str, Any]] = []
    page = 1
    while True:
        query = urllib.parse.urlencode(
            {**parameters, "per_page": str(MAX_PAGE_SIZE), "page": str(page)}
        )
        current_page = api_request(token, "GET", f"{path}?{query}")
        if not isinstance(current_page, list):
            raise RuntimeError(f"expected a list from GitHub API endpoint: {path}")
        results.extend(current_page)
        if len(current_page) < MAX_PAGE_SIZE:
            return results
        page += 1


def markdown_cell(value: object) -> str:
    """Keep untrusted analyzer text inside a single Markdown table cell."""

    return str(value or "").replace("|", "\\|").replace("\r", " ").replace("\n", " ")


def alert_severity(alert: dict[str, Any]) -> str:
    rule = alert.get("rule") or {}
    return str(rule.get("security_severity_level") or rule.get("severity") or "unknown")


def alert_sort_key(alert: dict[str, Any]) -> tuple[int, int]:
    ranks = {"critical": 0, "high": 1, "error": 1, "medium": 2, "warning": 2, "low": 3}
    return ranks.get(alert_severity(alert).lower(), 4), int(alert.get("number") or 0)


def build_open_report(repository: str, branch: str, alerts: list[dict[str, Any]]) -> str:
    rows: list[str] = []
    for alert in sorted(alerts, key=alert_sort_key):
        rule = alert.get("rule") or {}
        instance = alert.get("most_recent_instance") or {}
        location = instance.get("location") or {}
        message = instance.get("message") or {}
        path = location.get("path") or "unknown"
        start_line = location.get("start_line")
        location_text = f"`{path}{f':{start_line}' if start_line else ''}`"
        alert_number = int(alert.get("number") or 0)
        alert_url = alert.get("html_url") or (
            f"https://github.com/{repository}/security/code-scanning/{alert_number}"
        )
        rule_name = rule.get("name") or rule.get("id") or "Unnamed rule"
        rows.append(
            "| {severity} | [#{number} — {rule}]({url}) | {location} | {message} |".format(
                severity=markdown_cell(alert_severity(alert)),
                number=alert_number,
                rule=markdown_cell(rule_name),
                url=alert_url,
                location=markdown_cell(location_text),
                message=markdown_cell(message.get("text") or rule.get("description")),
            )
        )

    encoded_branch = urllib.parse.quote(branch, safe="")
    alerts_url = (
        f"https://github.com/{repository}/security/code-scanning"
        f"?query=is%3Aopen+branch%3A{encoded_branch}"
    )
    synchronized_at = dt.datetime.now(dt.UTC).replace(microsecond=0).isoformat()
    return "\n".join(
        [
            REPORT_MARKER,
            f"# CodeQL findings on `{branch}`",
            "",
            "> [!IMPORTANT]",
            "> This issue is generated from successful CodeQL analysis of the default branch.",
            "> Report suspected vulnerabilities privately as required by `SECURITY.md`.",
            "",
            f"**Open findings: {len(alerts)}**",
            "",
            "| Severity | Finding | Location | Description |",
            "| --- | --- | --- | --- |",
            *rows,
            "",
            f"[Open the live CodeQL alerts view]({alerts_url}).",
            "",
            f"Last synchronized: `{synchronized_at}`",
        ]
    )


def build_closed_report(repository: str, branch: str) -> str:
    synchronized_at = dt.datetime.now(dt.UTC).replace(microsecond=0).isoformat()
    return "\n".join(
        [
            REPORT_MARKER,
            f"# CodeQL findings on `{branch}`",
            "",
            "All CodeQL findings on the default branch have been fixed or otherwise closed.",
            "This tracker was closed automatically after a successful clean analysis.",
            "",
            f"Repository: `{repository}`",
            f"Last synchronized: `{synchronized_at}`",
        ]
    )


def find_tracker_issue(token: str, repository: str) -> dict[str, Any] | None:
    issues = paginated_get(
        token,
        f"/repos/{repository}/issues",
        {"state": "all", "sort": "updated", "direction": "desc"},
    )
    return next(
        (
            issue
            for issue in issues
            if "pull_request" not in issue and REPORT_MARKER in (issue.get("body") or "")
        ),
        None,
    )


def main() -> None:
    token = required_environment("GITHUB_TOKEN")
    repository = required_environment("GITHUB_REPOSITORY")
    repository_data = api_request(token, "GET", f"/repos/{repository}")
    branch = repository_data.get("default_branch")
    if not branch:
        raise RuntimeError("GitHub repository response did not include a default branch")

    alerts = paginated_get(
        token,
        f"/repos/{repository}/code-scanning/alerts",
        {
            "state": "open",
            "ref": f"refs/heads/{branch}",
            "tool_name": "CodeQL",
        },
    )
    tracker = find_tracker_issue(token, repository)
    title = f"[CodeQL] Findings on {branch}"

    if alerts:
        payload: dict[str, Any] = {
            "title": title,
            "body": build_open_report(repository, branch, alerts),
        }
        if tracker is None:
            created = api_request(token, "POST", f"/repos/{repository}/issues", payload)
            print(f"created CodeQL tracker issue #{created['number']} for {len(alerts)} alerts")
        else:
            payload["state"] = "open"
            if tracker.get("state") == "closed":
                payload["state_reason"] = "reopened"
            updated = api_request(
                token,
                "PATCH",
                f"/repos/{repository}/issues/{tracker['number']}",
                payload,
            )
            print(f"updated CodeQL tracker issue #{updated['number']} for {len(alerts)} alerts")
        return

    if tracker is None:
        print("no open CodeQL alerts and no tracker issue to close")
        return
    if tracker.get("state") == "closed":
        print(f"CodeQL tracker issue #{tracker['number']} is already closed")
        return

    closed = api_request(
        token,
        "PATCH",
        f"/repos/{repository}/issues/{tracker['number']}",
        {
            "title": title,
            "body": build_closed_report(repository, branch),
            "state": "closed",
            "state_reason": "completed",
        },
    )
    print(f"closed CodeQL tracker issue #{closed['number']} after a clean analysis")


if __name__ == "__main__":
    main()
