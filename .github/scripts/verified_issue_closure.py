"""Close explicitly accepted issues using trusted default-branch code and API data.

An untouched submitted COMMENT review on the PR that produced current main
consists of MARKER followed by a JSON object. Conversation comments never grant. Required
fields are schema, issue, issue_scope_sha256, complete, independent_review,
main_sha, base_sha, evidence, regions, and fixes. Each fix contains pr, head_sha,
review_base_sha, merge_sha, and evidence. Regions maps all supported versions to
evidence, objects_exact, source_linked, and final_dol_sha1 records. The maintainer
attests complete independent source/contract review and original-object/final-link
verification on the exact main/base; public proof comments must exist and be
authored by an authorized maintainer. The maintainer attests independent review;
checks supply a separate postmerge gate. No reference text implies acceptance.
"""

import argparse
import datetime
import hashlib
import json
import os
import re
import urllib.error
import urllib.parse
import urllib.request


REPOSITORY = "dev-camo/shadow-decomp"
REPOSITORY_ID = 1414155513
BOT_ID = 41898282  # github-actions[bot]
BUILD_WORKFLOW_ID = 381155316
REGIONS = ("GUPE8P", "GUPJ8P", "GUPP8P")
DOL_SHA1 = {
    "GUPE8P": "118ef49ebd45b371b6f514e0e1b6d1ba7cf12904",
    "GUPJ8P": "72f47bca7b72ec385a6fa30e123a96cc0fd19bab",
    "GUPP8P": "6ff04994371e93d6ba1015644f4e7467713b9363",
}
MARKER = "<!-- verified-issue-closure:v2 -->"
RECEIPT = "<!-- verified-issue-closure-receipt:v1:"
REVIEW_QUERY = """query IssueClosureReviews($owner: String!, $repo: String!, $number: Int!, $cursor: String) {
  repository(owner: $owner, name: $repo) {
    pullRequest(number: $number) {
      number headRefOid
      reviews(first: 100, after: $cursor) {
        totalCount pageInfo { hasNextPage endCursor }
        nodes { fullDatabaseId body state submittedAt lastEditedAt url commit { oid }
          author { __typename login ... on User { databaseId } } }
      }
    }
  }
}"""
SHA = re.compile(r"[0-9a-f]{40}\Z")
PROOF = re.compile(
    r"https://github\.com/dev-camo/shadow-decomp/(?:issues|pull)/[1-9][0-9]*"
    r"#(?:issuecomment|pullrequestreview)-[1-9][0-9]*\Z"
)


class Hold(Exception):
    """Evidence is absent, incomplete, stale, or cannot be verified."""


def require(condition, reason):
    if not condition:
        raise Hold(reason)


def digest(value):
    return hashlib.sha256(value.encode("utf-8")).hexdigest()


def scope_digest(issue):
    return digest(json.dumps({"title": issue["title"], "body": issue.get("body") or ""},
                             ensure_ascii=False, sort_keys=True, separators=(",", ":")))


def timestamp(value):
    return datetime.datetime.fromisoformat(value.replace("Z", "+00:00"))


def strict_object(pairs):
    result = {}
    for key, value in pairs:
        require(key not in result, "duplicate acceptance key")
        result[key] = value
    return result


class Api:
    """Fixed-origin client; one fixed GraphQL read, bounded REST, explicit writes."""

    def __init__(self, token):
        self.token = token

    def request(self, method, path, body=None):
        require(path.startswith("/") and not path.startswith("//"), "invalid API path")
        if path == "/graphql":
            require(method == "POST" and isinstance(body, dict) and set(body) == {"query", "variables"} and
                    body["query"] == REVIEW_QUERY, "only the fixed review-history query is permitted")
        data = None if body is None else json.dumps(body).encode("utf-8")
        request = urllib.request.Request(
            "https://api.github.com" + path,
            data=data,
            method=method,
            headers={
                "Authorization": "Bearer " + self.token,
                "Accept": "application/vnd.github+json",
                "X-GitHub-Api-Version": "2022-11-28",
                "Content-Type": "application/json",
                "User-Agent": "shadow-verified-issue-closure",
            },
        )
        # Never forward authorization to a redirected host.
        class NoRedirect(urllib.request.HTTPRedirectHandler):
            def redirect_request(self, *args, **kwargs):
                return None

        try:
            with urllib.request.build_opener(NoRedirect).open(request, timeout=30) as response:
                return json.load(response)
        except (urllib.error.URLError, ValueError) as error:
            raise Hold("GitHub API request failed: " + method) from error

    def get(self, path):
        return self.request("GET", path)

    def list(self, path, key=None):
        rows = []
        for page in range(1, 51):
            separator = "&" if "?" in path else "?"
            data = self.get(path + separator + urllib.parse.urlencode({"per_page": 100, "page": page}))
            batch = data if key is None else data[key]
            require(isinstance(batch, list), "invalid paginated API response")
            rows.extend(batch)
            if len(batch) < 100:
                # Check APIs cap their result set; never accept a truncated set.
                if key and "total_count" in data:
                    require(data["total_count"] <= len(rows), "truncated API collection")
                return rows
        raise Hold("API pagination limit exceeded")


class Gate:
    def __init__(self, api, authorization=None):
        self.api = api
        self.root = "/repos/" + REPOSITORY
        self.proofs = {}
        policy = authorization if authorization is not None else json.loads(
            os.environ["ISSUE_CLOSURE_AUTHORIZATION"], object_pairs_hook=strict_object)
        require(set(policy) == {"implementation_authors", "verification_authors"}, "invalid authorization configuration")
        allowed = []
        for name in ("implementation_authors", "verification_authors"):
            require(isinstance(policy[name], list) and policy[name], "empty authorization configuration")
            identities = {}
            for user in policy[name]:
                require(set(user) == {"id", "login"} and type(user["id"]) is int and user["id"] > 0 and
                        isinstance(user["login"], str) and re.fullmatch(r"[A-Za-z0-9-]{1,39}", user["login"]) and
                        user["id"] not in identities, "invalid authorization identity")
                identities[user["id"]] = user["login"]
            allowed.append(identities)
        self.implementation_authors, self.verification_authors = allowed
        require(not self.implementation_authors.keys() & self.verification_authors.keys() and
                not set(self.implementation_authors.values()) & set(self.verification_authors.values()),
                "implementation and verification authorization must be separate")

    def trusted(self, comment, actors=None):
        actors = self.verification_authors if actors is None else actors
        user = comment.get("user") or {}
        return (user.get("type") == "User" and
                actors.get(user.get("id")) == user.get("login"))

    def authorize(self, user, actors, role):
        require(self.trusted({"user": user}, actors), "unauthorized " + role + " identity")
        permission = self.api.get(self.root + "/collaborators/" + user["login"] + "/permission")
        require(permission.get("permission") in ("admin", "write") and
                self.trusted({"user": permission["user"]}, actors) and permission["user"]["id"] == user["id"],
                role + " lacks current write access")

    def anchor(self, main):
        candidates = []
        for linked in self.api.list(self.root + "/commits/" + main + "/pulls"):
            pr = self.api.get(self.root + "/pulls/" + str(linked["number"]))
            if (pr["merged"] is True and pr["state"] == "closed" and pr["merge_commit_sha"] == main and
                    pr["base"]["repo"]["id"] == REPOSITORY_ID and pr["base"]["ref"] == "main"):
                candidates.append(pr)
        require(len(candidates) == 1, "current main acceptance anchor missing or ambiguous")
        return candidates[0]

    def reviews(self, number, head=None):
        owner, repo = REPOSITORY.split("/", 1)
        rows, cursor, cursors = [], None, set()
        for _ in range(50):
            data = self.api.request("POST", "/graphql", {"query": REVIEW_QUERY, "variables": {
                "owner": owner, "repo": repo, "number": number, "cursor": cursor}})
            require(isinstance(data, dict) and not data.get("errors"), "review history read failed or partial")
            pull = data["data"]["repository"]["pullRequest"]
            require(pull["number"] == number and (head is None or pull["headRefOid"] == head),
                    "review history anchor/head mismatch")
            connection = pull["reviews"]
            batch = connection["nodes"]
            require(isinstance(batch, list), "review history unavailable")
            for node in batch:
                require(isinstance(node, dict) and set(node) >= {"fullDatabaseId", "body", "state", "submittedAt",
                        "lastEditedAt", "url", "commit", "author"}, "review edit/submission metadata missing")
                identity = node["fullDatabaseId"]
                require(type(identity) is int or (isinstance(identity, str) and identity.isdecimal()), "invalid review ID")
                require(int(identity) > 0 and (node["lastEditedAt"] is None or isinstance(node["lastEditedAt"], str)),
                        "invalid review edit/identity metadata")
                author = node["author"] or {}
                rows.append({"id": int(identity), "body": node["body"], "state": node["state"],
                    "submitted_at": node["submittedAt"], "last_edited_at": node["lastEditedAt"],
                    "html_url": node["url"], "commit_id": (node["commit"] or {}).get("oid"),
                    "user": {"id": author.get("databaseId"), "login": author.get("login"), "type": author.get("__typename")}})
            page = connection["pageInfo"]
            if not page["hasNextPage"]:
                require(type(connection["totalCount"]) is int and connection["totalCount"] == len(rows) and
                        len({r["id"] for r in rows}) == len(rows), "review history truncated or duplicate")
                return rows
            cursor = page["endCursor"]
            require(isinstance(cursor, str) and cursor and cursor not in cursors, "review pagination incomplete")
            cursors.add(cursor)
        raise Hold("review history pagination limit exceeded")

    def acceptance(self, issue, history, anchor):
        submitted = []
        for review in history:
            if review["state"] == "PENDING":
                require(review["submitted_at"] is None, "invalid pending review metadata")
                continue
            require(review["state"] in ("COMMENTED", "APPROVED", "CHANGES_REQUESTED", "DISMISSED") and
                    isinstance(review["submitted_at"], str), "incomplete submitted history")
            submitted.append(review)
        require(submitted, "no explicit submitted acceptance review")
        comment = max(submitted, key=lambda r: (timestamp(r["submitted_at"]), r["id"]))
        require(sum(r["submitted_at"] == comment["submitted_at"] for r in submitted) == 1,
                "submitted review time ambiguity; fresh acceptance required")
        require(comment["state"] == "COMMENTED" and comment["last_edited_at"] is None,
                "latest submitted review edited or dismissed; fresh acceptance required")
        require(not any(r["last_edited_at"] is not None and
                        timestamp(r["last_edited_at"]) >= timestamp(comment["submitted_at"]) for r in submitted),
                "submitted history edited at/after acceptance; fresh acceptance required")
        require(self.trusted(comment), "latest submitted review lacks verification authority")
        require(comment["commit_id"] == anchor["head"]["sha"] and
                timestamp(comment["submitted_at"]) > timestamp(anchor["merged_at"]),
                "acceptance must follow actual anchor merge on its exact reviewed head")
        self.acceptance_time = timestamp(comment["submitted_at"])
        self.authorize(comment["user"], self.verification_authors, "acceptance author")
        body = (comment["body"] or "").strip()
        require(body.startswith(MARKER), "acceptance marker must begin review")
        try:
            record = json.loads(body[len(MARKER):].strip(), object_pairs_hook=strict_object)
        except (ValueError, TypeError) as error:
            raise Hold("malformed acceptance JSON") from error
        keys = {"schema", "issue", "issue_scope_sha256", "complete", "independent_review",
                "main_sha", "base_sha", "evidence", "regions", "fixes"}
        require(isinstance(record, dict) and set(record) == keys, "invalid acceptance fields")
        require(type(record["schema"]) is int and record["schema"] == 2 and
                type(record["issue"]) is int and record["issue"] == issue["number"], "wrong acceptance issue/schema")
        require(record["complete"] is True and record["independent_review"] is True,
                "complete independent issue acceptance required")
        require(record["issue_scope_sha256"] == scope_digest(issue), "issue title/body scope changed")
        require(isinstance(record["regions"], dict) and set(record["regions"]) == set(REGIONS),
                "all regional proofs required")
        require(isinstance(record["fixes"], list) and 0 < len(record["fixes"]) <= 25, "invalid fixing PR list")
        self.proof(record["evidence"])
        for region, proof in record["regions"].items():
            require(isinstance(proof, dict) and set(proof) ==
                    {"evidence", "objects_exact", "source_linked", "final_dol_sha1"}, "invalid regional proof")
            require(proof["objects_exact"] is True and proof["source_linked"] is True and
                    proof["final_dol_sha1"] == DOL_SHA1[region], "regional object/source/final verification incomplete")
            self.proof(proof["evidence"])
        for field in ("main_sha", "base_sha"):
            require(isinstance(record[field], str) and SHA.fullmatch(record[field]), "invalid main/base SHA")
        numbers = set()
        for fix in record["fixes"]:
            require(isinstance(fix, dict) and set(fix) ==
                    {"pr", "head_sha", "review_base_sha", "merge_sha", "evidence"}, "invalid fix fields")
            require(type(fix["pr"]) is int and fix["pr"] > 0 and fix["pr"] not in numbers,
                    "duplicate or invalid fixing PR")
            numbers.add(fix["pr"])
            for field in ("head_sha", "review_base_sha", "merge_sha"):
                require(isinstance(fix[field], str) and SHA.fullmatch(fix[field]), "invalid fix SHA")
            self.proof(fix["evidence"])
        return comment, record

    def proof(self, value):
        require(isinstance(value, str) and PROOF.fullmatch(value), "public local evidence permalink required")
        if value in self.proofs:
            return
        path, anchor = value.split("#", 1)
        number = path.rsplit("/", 1)[1]
        kind, identity = anchor.rsplit("-", 1)
        if kind == "issuecomment":
            proof = self.api.get(self.root + "/issues/comments/" + identity)
        else:
            require("/pull/" in path, "review proof must refer to a pull request")
            matches = [r for r in self.reviews(int(number)) if r["id"] == int(identity)]
            require(len(matches) == 1, "review proof missing or ambiguous")
            proof = matches[0]
            require(proof.get("state") in ("APPROVED", "COMMENTED"), "review proof dismissed or not accepted")
        require(proof["html_url"] == value and self.trusted(proof) and (proof.get("body") or "").strip(),
                "proof missing or not authored by authorized verification author")
        self.authorize(proof["user"], self.verification_authors, "proof author")
        changed = proof.get("last_edited_at") or proof.get("updated_at") or proof["submitted_at"]
        require(timestamp(changed) <= self.acceptance_time, "proof changed after acceptance")
        self.proofs[value] = {"url": value, "body_sha256": digest(proof["body"]),
                              "changed_at": changed}

    def policy(self):
        repo = self.api.get(self.root)
        require(repo["id"] == REPOSITORY_ID and repo["full_name"] == REPOSITORY and
                repo["default_branch"] == "main", "repository/default branch changed")
        branch = self.api.get(self.root + "/branches/main")
        summary = branch.get("protection", {}).get("required_status_checks")
        require(isinstance(summary, dict) and isinstance(summary.get("contexts"), list) and
                isinstance(summary.get("checks"), list), "required check policy unavailable")
        required = {(c["context"], c.get("app_id")) for c in summary["checks"]}
        known = {name for name, _ in required}
        required.update((name, None) for name in summary["contexts"] if name not in known)
        for rule in self.api.list(self.root + "/rules/branches/main"):
            require(rule["type"] not in ("workflows", "required_workflows"), "required workflow rule unsupported")
            if rule["type"] == "required_status_checks":
                required.update((c["context"], c.get("integration_id"))
                                for c in rule["parameters"]["required_status_checks"])
        for name, app in required:
            require(isinstance(name, str) and name and (app is None or type(app) is int), "invalid check policy")
        return branch["commit"]["sha"], sorted(required, key=lambda c: (c[0], c[1] or -1))

    def checks(self, sha, required):
        checks = self.api.list(self.root + "/commits/" + sha + "/check-runs?filter=all", "check_runs")
        statuses = self.api.list(self.root + "/commits/" + sha + "/statuses")
        latest_checks = {}
        for check in checks:
            require(check["head_sha"] == sha, "check SHA mismatch")
            key = (check["name"], check["app"]["id"])
            if key not in latest_checks or check["id"] > latest_checks[key]["id"]:
                latest_checks[key] = check
        latest_statuses = {}
        for status in statuses:
            name = status["context"]
            if name not in latest_statuses or status["id"] > latest_statuses[name]["id"]:
                latest_statuses[name] = status
        evidence = []
        for name, app in required:
            selected = [c for (context, source), c in latest_checks.items()
                        if context == name and (app in (None, -1) or source == app)]
            status = latest_statuses.get(name) if app in (None, -1) else None
            require(selected or status, "required check missing: " + name)
            require(all(c["status"] == "completed" and c["conclusion"] == "success" for c in selected) and
                    (not status or status["state"] == "success"), "required check not successful: " + name)
            evidence.extend({"check": c["name"], "id": c["id"], "app": c["app"]["id"]} for c in selected)
            if status:
                evidence.append({"status": name, "id": status["id"]})
        runs = self.api.list(self.root + "/actions/workflows/" + str(BUILD_WORKFLOW_ID) +
                             "/runs?" + urllib.parse.urlencode({"head_sha": sha, "event": "push"}), "workflow_runs")
        runs = [r for r in runs if r["head_sha"] == sha and r["head_branch"] == "main" and
                r["event"] == "push" and r["head_repository"]["id"] == REPOSITORY_ID and
                r["workflow_id"] == BUILD_WORKFLOW_ID and r["path"] == ".github/workflows/build.yml"]
        require(runs, "merged/main push Build missing")
        run = max(runs, key=lambda r: r["id"])
        require(run["status"] == "completed" and run["conclusion"] == "success", "latest push Build not successful")
        # A failed-job-only rerun omits successful carried-forward jobs from its
        # attempt. Use all attempts of this same run; the latest execution of each
        # job must succeed, never a prior success hiding a newer failure.
        history = self.api.list(self.root + "/actions/runs/" + str(run["id"]) + "/jobs?filter=all", "jobs")
        jobs, seen = {}, set()
        for job in history:
            require(job["run_id"] == run["id"] and job["head_sha"] == sha and
                    type(job["run_attempt"]) is int and 1 <= job["run_attempt"] <= run["run_attempt"],
                    "Build job run/SHA/attempt mismatch")
            key = (job["name"], job["run_attempt"])
            require(key not in seen, "regional Build job missing/ambiguous")
            seen.add(key)
            if job["name"] not in jobs or job["run_attempt"] > jobs[job["name"]]["run_attempt"]:
                jobs[job["name"]] = job
        require(any(j["run_attempt"] == run["run_attempt"] for j in jobs.values()),
                "latest Build attempt has no jobs")
        require(all(j["status"] == "completed" and j["conclusion"] == "success" for j in jobs.values()),
                "Build contains incomplete or unsuccessful jobs")
        for region in REGIONS:
            require("build (" + region + ")" in jobs,
                    "regional Build job missing/ambiguous: " + region)
        return {"sha": sha, "required": evidence, "build_run": run["id"],
                "attempt": run["run_attempt"], "jobs": [
                    {"id": j["id"], "name": j["name"], "attempt": j["run_attempt"]}
                    for j in sorted(jobs.values(), key=lambda j: j["name"])]}

    def verify_runs(self, checks):
        for check in checks:
            run = self.api.get(self.root + "/actions/runs/" + str(check["build_run"]))
            require(run["id"] == check["build_run"] and run["head_sha"] == check["sha"] and
                    run["head_branch"] == "main" and run["event"] == "push" and
                    run["head_repository"]["id"] == REPOSITORY_ID and run["workflow_id"] == BUILD_WORKFLOW_ID and
                    run["path"] == ".github/workflows/build.yml" and run["status"] == "completed" and
                    run["conclusion"] == "success" and run["run_attempt"] == check["attempt"],
                    "Build provenance/status/attempt changed during qualification")

    def snapshot(self, number):
        self.proofs = {}
        issue = self.api.get(self.root + "/issues/" + str(number))
        require(issue["state"] == "open" and "pull_request" not in issue, "issue not open")
        comments = self.api.list(self.root + "/issues/" + str(number) + "/comments")
        main, required = self.policy()
        anchor = self.anchor(main)
        self.authorize(anchor["user"], self.implementation_authors, "implementation author")
        history = self.reviews(anchor["number"], anchor["head"]["sha"])
        comment, record = self.acceptance(issue, history, anchor)
        timeline = self.api.list(self.root + "/issues/" + str(number) + "/timeline")
        require(not any(t["event"] == "reopened" and timestamp(t["created_at"]) >= timestamp(comment["submitted_at"])
                        for t in timeline), "issue reopened; fresh acceptance required")
        require(main == record["main_sha"], "tested main is stale")
        commit = self.api.get(self.root + "/commits/" + main)
        require(len(commit["parents"]) == 1 and commit["parents"][0]["sha"] == record["base_sha"],
                "tested main base mismatch")
        shas = {main}
        for fix in record["fixes"]:
            pr = self.api.get(self.root + "/pulls/" + str(fix["pr"]))
            self.authorize(pr["user"], self.implementation_authors, "implementation author")
            require(pr["user"]["id"] != comment["user"]["id"], "implementation author cannot accept own work")
            require(pr["merged"] is True and pr["state"] == "closed" and
                    pr["base"]["repo"]["id"] == REPOSITORY_ID and pr["base"]["ref"] == "main", "fix not merged into main")
            require(pr["head"]["sha"] == fix["head_sha"] and pr["base"]["sha"] == fix["review_base_sha"] and
                    pr["merge_commit_sha"] == fix["merge_sha"], "fix head/review base/merge SHA changed")
            require(timestamp(comment["submitted_at"]) > timestamp(pr["merged_at"]), "acceptance predates merged fix")
            compare = self.api.get(self.root + "/compare/" + fix["merge_sha"] + "..." + main)
            require(compare["status"] in ("ahead", "identical") and
                    compare["merge_base_commit"]["sha"] == fix["merge_sha"], "merged fix not ancestor of tested main")
            shas.add(fix["merge_sha"])
        checks = [self.checks(sha, required) for sha in sorted(shas)]
        self.verify_runs(checks)
        key = str(comment["id"]) + ":" + digest(comment["body"])
        receipt_marker = RECEIPT + key + " -->"
        result = {"issue": number, "acceptance": comment["html_url"], "acceptance_key": key,
                "anchor": {"pr": anchor["number"], "head_sha": anchor["head"]["sha"], "merge_sha": main},
                "review_history_sha256": digest(json.dumps(sorted(history, key=lambda r: r["id"]), sort_keys=True)),
                "record": record, "checks": checks, "policy": required,
                "receipt_marker": receipt_marker, "proofs": sorted(self.proofs.values(), key=lambda p: p["url"])}
        result["receipts"] = [c for c in comments if c.get("user", {}).get("id") == BOT_ID and
                              c["user"].get("type") == "Bot" and (c.get("body") or "").startswith(receipt_marker)]
        return result

    @staticmethod
    def receipt_body(snapshot):
        receipt = snapshot["receipt_marker"] + "\nVerified complete issue acceptance: " + snapshot["acceptance"]
        receipt += "\n\nMerged revisions and successful postmerge checks:\n\n"
        receipt += "```json\n" + json.dumps({k: snapshot[k] for k in ("anchor", "review_history_sha256", "record", "proofs", "checks", "policy")}, sort_keys=True, indent=2)
        return receipt + "\n```"

    def run(self, number, apply=False):
        snapshot = self.snapshot(number)
        if not apply:
            return {"issue": number, "result": "would close", "evidence": snapshot}
        fresh = self.snapshot(number)
        require(snapshot == fresh, "evidence/checks/policy changed before receipt")
        if not fresh["receipts"]:
            self.api.request("POST", self.root + "/issues/" + str(number) + "/comments", {"body": self.receipt_body(fresh)})
        else:
            receipt = min(fresh["receipts"], key=lambda c: c["id"])
            if receipt["body"] != self.receipt_body(fresh):
                self.api.request("PATCH", self.root + "/issues/comments/" + str(receipt["id"]),
                                 {"body": self.receipt_body(fresh)})
        before_close = self.snapshot(number)
        require({k: v for k, v in fresh.items() if k != "receipts"} ==
                {k: v for k, v in before_close.items() if k != "receipts"}, "evidence changed before closure")
        require(any(c["body"] == self.receipt_body(before_close) for c in before_close["receipts"]),
                "closure receipt missing or stale")
        self.verify_runs(before_close["checks"])
        self.api.request("PATCH", self.root + "/issues/" + str(number), {"state": "closed", "state_reason": "completed"})
        return {"issue": number, "result": "closed", "acceptance": fresh["acceptance"]}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--issue", type=int)
    parser.add_argument("--apply", action="store_true", help="write only after complete verification")
    args = parser.parse_args()
    require(args.issue is None or args.issue > 0, "invalid issue number")
    require(os.environ.get("GITHUB_REPOSITORY") == REPOSITORY, "wrong workflow repository")
    require(os.environ.get("GITHUB_REF") == "refs/heads/main", "trusted main execution required")
    require(not args.apply or os.environ.get("VERIFIED_ISSUE_CLOSURE_ENABLED") == "true", "closure is disabled")
    api = Api(os.environ["GITHUB_TOKEN"])
    gate = Gate(api)
    issues = ([{"number": args.issue}] if args.issue else
              api.list(gate.root + "/issues?state=open&sort=created&direction=asc"))
    for issue in issues:
        if "pull_request" in issue:
            continue
        try:
            print(json.dumps(gate.run(issue["number"], apply=args.apply), sort_keys=True))
        except (Hold, KeyError, TypeError, ValueError) as error:
            # A malformed/incomplete issue cannot stop unrelated accepted issues.
            print(json.dumps({"issue": issue["number"], "result": "held", "reason": str(error)}))


if __name__ == "__main__":
    main()
