"""Deterministic in-memory fixtures: no network, credentials, or live issue writes."""

import copy
import json
import os
import pathlib
import subprocess
import sys
import unittest
import urllib.error
import urllib.parse
from unittest import mock

import verified_issue_closure as closure


MAIN = "a" * 40
BASE = "b" * 40
HEAD = "c" * 40
REVIEW_BASE = "d" * 40
MERGE = "e" * 40
SECOND_MERGE = "f" * 40
TIME = "2026-10-14T12:00:00Z"
EARLIER = "2026-10-14T11:00:00Z"
LATER = "2026-10-14T13:00:00Z"
PROOF = "https://github.com/dev-camo/shadow-decomp/pull/10#issuecomment-200"
VERIFICATION_AUTHOR = {"id": 20202, "login": "verification-author", "type": "User"}
IMPLEMENTATION_AUTHOR = {"id": 10101, "login": "implementation-author", "type": "User"}


class Fixture:
    def __init__(self):
        self.root = "/repos/" + closure.REPOSITORY
        self.issue = {"number": 1, "state": "open", "title": "Accepted issue scope",
                      "body": "Acceptance checks for the entire issue."}
        self.record = {
            "schema": 2, "issue": 1, "issue_scope_sha256": closure.scope_digest(self.issue),
            "complete": True, "independent_review": True, "main_sha": MAIN, "base_sha": BASE,
            "evidence": PROOF, "regions": {r: {"evidence": PROOF, "objects_exact": True,
                "source_linked": True, "final_dol_sha1": closure.DOL_SHA1[r]} for r in closure.REGIONS},
            "fixes": [{"pr": 10, "head_sha": HEAD, "review_base_sha": REVIEW_BASE, "merge_sha": MERGE, "evidence": PROOF}],
        }
        self.comment = {"id": 100, "user": VERIFICATION_AUTHOR, "created_at": TIME, "updated_at": TIME,
                        "submitted_at": TIME, "last_edited_at": None, "state": "COMMENTED", "commit_id": "9" * 40,
                        "html_url": "https://github.com/dev-camo/shadow-decomp/pull/20#pullrequestreview-100"}
        self.refresh()
        self.authorization = {"implementation_authors": [{"id": IMPLEMENTATION_AUTHOR["id"], "login": IMPLEMENTATION_AUTHOR["login"]}],
                              "verification_authors": [{"id": VERIFICATION_AUTHOR["id"], "login": VERIFICATION_AUTHOR["login"]}]}
        self.reviews = [self.comment]
        self.proof_reviews = {}
        self.comments = []
        self.graph_error = False
        self.graph_transform = None
        self.anchor_prs = [{"number": 20}]
        self.timeline = []
        self.repo = {"id": closure.REPOSITORY_ID, "full_name": closure.REPOSITORY, "default_branch": "main"}
        self.branch = {"commit": {"sha": MAIN}, "protection": {"required_status_checks": {"contexts": [], "checks": []}}}
        self.rules = []
        self.permission = {"permission": "write", "user": VERIFICATION_AUTHOR}
        self.implementation_permission = {"permission": "admin", "user": IMPLEMENTATION_AUTHOR}
        self.proof = {"id": 200, "user": VERIFICATION_AUTHOR, "body": "Independent head/base and full issue/main/regional verification.",
                      "html_url": PROOF, "updated_at": EARLIER}
        self.prs = {10: {"user": IMPLEMENTATION_AUTHOR, "merged": True, "state": "closed", "merged_at": EARLIER, "merge_commit_sha": MERGE,
                        "head": {"sha": HEAD}, "base": {"ref": "main", "sha": REVIEW_BASE,
                            "repo": {"id": closure.REPOSITORY_ID}}}}
        self.prs[20] = {**copy.deepcopy(self.prs[10]), "number": 20, "merge_commit_sha": MAIN,
                       "head": {"sha": "9" * 40}}
        self.runs = {}
        self.jobs = {}
        self.checks = {MAIN: [], MERGE: []}
        self.statuses = {MAIN: [], MERGE: []}
        for index, sha in enumerate((MAIN, MERGE), 1):
            self.add_build(sha, index)
        self.writes = []
        self.read_hook = None
        self.fail_close = False
        self.compare = "ahead"

    def refresh(self):
        self.comment["body"] = closure.MARKER + "\n" + json.dumps(self.record)

    def delete_review(self, review):
        # Model GitHub's documented normal review deletion boundary.
        if review["state"] != "PENDING":
            raise closure.Hold("submitted reviews cannot be deleted")
        self.reviews.remove(review)

    def add_build(self, sha, identity):
        self.runs.setdefault(sha, []).append({"id": identity, "status": "completed", "conclusion": "success",
            "head_sha": sha, "head_branch": "main", "event": "push", "run_attempt": 1,
            "workflow_id": closure.BUILD_WORKFLOW_ID, "path": ".github/workflows/build.yml",
            "head_repository": {"id": closure.REPOSITORY_ID}})
        self.jobs[(identity, 1)] = [{"id": identity * 100 + n, "name": "build (" + region + ")",
                                   "run_id": identity, "run_attempt": 1, "head_sha": sha,
                                   "status": "completed", "conclusion": "success"}
                                  for n, region in enumerate(closure.REGIONS)]

    def get(self, path):
        if self.read_hook:
            self.read_hook(path)
        local = path.removeprefix(self.root)
        if local == "":
            value = self.repo
        elif local == "/issues/1":
            value = self.issue
        elif local == "/branches/main":
            value = self.branch
        elif local == "/collaborators/implementation-author/permission":
            value = self.implementation_permission
        elif local == "/collaborators/verification-author/permission":
            value = self.permission
        elif local.startswith("/issues/comments/") or "/reviews/" in local:
            value = self.proof
        elif local.startswith("/commits/"):
            value = {"parents": [{"sha": BASE}]}
        elif local.startswith("/actions/runs/"):
            identity = int(local.rsplit("/", 1)[1])
            value = next(r for runs in self.runs.values() for r in runs if r["id"] == identity)
        elif local.startswith("/pulls/"):
            value = self.prs[int(local.rsplit("/", 1)[1])]
        elif local.startswith("/compare/"):
            value = {"status": self.compare, "merge_base_commit": {"sha": local.split("/")[-1].split("...")[0]}}
        else:
            raise AssertionError("unexpected GET " + path)
        return copy.deepcopy(value)

    def list(self, path, key=None):
        if self.read_hook:
            self.read_hook(path)
        local = path.removeprefix(self.root)
        if local == "/issues/1/comments":
            value = self.comments
        elif local == "/issues/1/timeline":
            value = self.timeline
        elif local == "/rules/branches/main":
            value = self.rules
        elif local.endswith("/check-runs?filter=all"):
            value = self.checks[local.split("/")[2]]
        elif local.endswith("/statuses"):
            value = self.statuses[local.split("/")[2]]
        elif local.endswith("/pulls"):
            value = self.anchor_prs
        elif local.startswith("/actions/workflows/"):
            value = self.runs[urllib.parse.parse_qs(urllib.parse.urlsplit(local).query)["head_sha"][0]]
        elif local.startswith("/actions/runs/"):
            parts = local.split("/")
            identity = int(parts[3])
            value = [j for (run, _), jobs in self.jobs.items() if run == identity for j in jobs]
        else:
            raise AssertionError("unexpected list " + path)
        return copy.deepcopy(value)

    def request(self, method, path, body=None):
        if path == "/graphql":
            if method != "POST" or body["query"] != closure.REVIEW_QUERY:
                raise closure.Hold("only fixed history reads are supported")
            if self.graph_error:
                return {"errors": [{"message": "read failed"}]}
            number = body["variables"]["number"]
            history = self.reviews if number == 20 else self.proof_reviews.get(number, [])
            nodes = [{"fullDatabaseId": str(r["id"]), "body": r["body"], "state": r["state"],
                      "submittedAt": r["submitted_at"], "lastEditedAt": r["last_edited_at"],
                      "url": r["html_url"], "commit": {"oid": r["commit_id"]},
                      "author": {"__typename": r["user"]["type"], "login": r["user"]["login"],
                                 "databaseId": r["user"]["id"]}} for r in history]
            result = {"data": {"repository": {"pullRequest": {"number": number, "headRefOid": self.prs[number]["head"]["sha"],
                      "reviews": {"totalCount": len(nodes),
                      "nodes": nodes, "pageInfo": {"hasNextPage": False, "endCursor": None}}}}}}
            return self.graph_transform(result) if self.graph_transform else copy.deepcopy(result)
        if method == "PATCH" and path.endswith("/issues/1") and self.fail_close:
            raise closure.Hold("simulated closure failure")
        self.writes.append((method, path, copy.deepcopy(body)))
        if method == "POST":
            self.comments.append({"id": max([100] + [c["id"] for c in self.comments]) + 1, "body": body["body"], "user": {"id": closure.BOT_ID, "type": "Bot"},
                                  "created_at": TIME, "updated_at": TIME})
        elif method == "PATCH" and "/issues/comments/" in path:
            identity = int(path.rsplit("/", 1)[1])
            next(c for c in self.comments if c["id"] == identity).update(body)
        elif method == "PATCH":
            self.issue.update(body)
        else:
            raise AssertionError("unexpected mutation")
        return {}


class ClosureTests(unittest.TestCase):
    def setUp(self):
        self.f = Fixture()
        self.gate = closure.Gate(self.f, self.f.authorization)

    def held(self, reason):
        closures = sum(w[0] == "PATCH" and w[1].endswith("/issues/1") for w in self.f.writes)
        with self.assertRaisesRegex(closure.Hold, reason):
            self.gate.run(1, apply=True)
        self.assertEqual(sum(w[0] == "PATCH" and w[1].endswith("/issues/1") for w in self.f.writes), closures)

    def withdrawal(self, state="COMMENTED", body="Acceptance withdrawn."):
        review = {**copy.deepcopy(self.f.comment), "id": 101, "submitted_at": LATER,
                  "state": state, "body": body}
        self.f.reviews.append(review)
        return review

    def test_observed_and_unobserved_withdrawals_remain_retained(self):
        for observed in (False, True):
            with self.subTest(observed=observed):
                self.setUp()
                review = self.withdrawal()
                if observed:
                    self.held("marker must begin")
                # Submitted reviews are not deletable through the documented API.
                with self.assertRaisesRegex(closure.Hold, "cannot be deleted"):
                    self.f.delete_review(review)
                review["body"] = ""
                review["last_edited_at"] = LATER
                self.gate = closure.Gate(self.f, self.f.authorization)
                self.held("edited or dismissed")
                self.assertEqual(self.f.writes, [])

    def test_dismissal_retains_barrier_and_fresh_submitted_acceptance_recovers(self):
        self.withdrawal(state="DISMISSED", body="")
        self.held("edited or dismissed")
        self.f.reviews.append({**copy.deepcopy(self.f.comment), "id": 102,
            "submitted_at": "2026-10-14T14:00:00Z"})
        self.assertEqual(self.gate.run(1, True)["result"], "closed")

    def test_same_second_body_edit_is_rejected_by_explicit_edit_metadata(self):
        self.f.comment["last_edited_at"] = self.f.comment["submitted_at"]
        self.held("edited")

    def test_older_submitted_row_edited_after_grant_requires_fresh_acceptance(self):
        old = {**copy.deepcopy(self.f.comment), "id": 99, "submitted_at": EARLIER,
               "body": "Earlier submitted discussion.", "last_edited_at": LATER}
        self.f.reviews.insert(0, old)
        self.held("history edited at/after acceptance")
        self.f.reviews.append({**copy.deepcopy(self.f.comment), "id": 102,
            "submitted_at": "2026-10-14T14:00:00Z"})
        self.assertEqual(self.gate.run(1)["result"], "would close")

    def test_old_created_but_later_submitted_withdrawal_blocks(self):
        review = self.withdrawal()
        review["created_at"] = EARLIER
        self.held("marker must begin")

    def test_pending_review_deletion_never_grants_or_revokes(self):
        pending = self.withdrawal(state="PENDING")
        pending["submitted_at"] = None
        self.assertEqual(self.gate.run(1)["result"], "would close")
        self.f.delete_review(pending)
        self.assertEqual(self.gate.run(1)["result"], "would close")
        self.f.reviews.clear()
        self.held("no explicit submitted")

    def test_canceled_replaced_and_overflowed_wakeups_cannot_erase_review_barrier(self):
        for outcome in ("canceled", "replaced", "overflowed", "failed"):
            with self.subTest(outcome=outcome):
                self.setUp()
                self.withdrawal()
                # No event payload is passed; fresh qualification rereads durable reviews.
                self.gate = closure.Gate(self.f, self.f.authorization)
                self.held("marker must begin")
                self.assertEqual(self.f.writes, [])

    def test_read_failure_before_mutation_and_later_retry_keep_withdrawal(self):
        self.withdrawal()
        self.f.graph_error = True
        self.held("read failed or partial")
        self.assertEqual(self.f.writes, [])
        self.f.graph_error = False
        self.gate = closure.Gate(self.f, self.f.authorization)
        self.held("marker must begin")

    def test_no_invalidation_patch_is_needed_even_when_state_writes_would_fail(self):
        self.withdrawal()
        request = self.f.request
        def reject_state_write(method, path, body=None):
            if method == "PATCH" and "/issues/comments/" in path:
                raise closure.Hold("failure before state mutation")
            return request(method, path, body)
        self.f.request = reject_state_write
        self.held("marker must begin")
        self.gate = closure.Gate(self.f, self.f.authorization)
        self.held("marker must begin")
        self.assertEqual(self.f.writes, [])

    def test_partial_missing_and_wrong_anchor_graphql_data_fail_closed(self):
        for defect in ("errors", "count", "edit", "anchor"):
            with self.subTest(defect=defect):
                self.setUp()
                def corrupt(data):
                    pull = data["data"]["repository"]["pullRequest"]
                    if defect == "errors":
                        data["errors"] = [{"message": "partial"}]
                    elif defect == "count":
                        pull["reviews"]["totalCount"] += 1
                    elif defect == "edit":
                        del pull["reviews"]["nodes"][0]["lastEditedAt"]
                    else:
                        pull["number"] = 99
                    return data
                self.f.graph_transform = corrupt
                self.held("partial|truncated|metadata missing|anchor/head mismatch")

    def test_anchor_submission_head_merge_time_and_tied_reviews_are_checked(self):
        self.f.comment["commit_id"] = HEAD
        self.held("exact reviewed head")
        self.f.comment["commit_id"] = "9" * 40
        self.f.comment["submitted_at"] = EARLIER
        self.held("follow actual anchor merge")
        self.f.comment["submitted_at"] = TIME
        self.f.reviews.append({**copy.deepcopy(self.f.comment), "id": 101})
        self.held("time ambiguity")

    def test_untrusted_later_submitted_review_holds_without_granting_authority(self):
        review = self.withdrawal(body=self.f.comment["body"])
        review["user"] = IMPLEMENTATION_AUTHOR
        self.held("verification authority")

    def test_legacy_conversation_grants_are_never_eligible(self):
        self.f.reviews.clear()
        self.f.comments = [{**copy.deepcopy(self.f.comment), "body": self.f.comment["body"].replace(":v2", ":v1")}]
        self.held("no explicit submitted")

    def test_changed_or_disappearing_captured_history_is_rejected_before_close(self):
        count = [0]
        def hook(path):
            if path == self.f.root + "/issues/1":
                count[0] += 1
                if count[0] == 3:
                    self.withdrawal(state="DISMISSED")
        self.f.read_hook = hook
        self.held("edited or dismissed")
        self.assertEqual([w[0] for w in self.f.writes], ["POST"])

    def test_previously_captured_row_disappearance_or_alteration_holds(self):
        for defect in ("remove", "alter"):
            with self.subTest(defect=defect):
                self.setUp()
                old = {**copy.deepcopy(self.f.comment), "id": 99, "submitted_at": EARLIER,
                       "body": "Old submitted discussion."}
                self.f.reviews.insert(0, old)
                count = [0]
                def hook(path):
                    if path == self.f.root + "/issues/1":
                        count[0] += 1
                        if count[0] == 3:
                            # Simulate incomplete service data/platform moderation, not normal deletion.
                            if defect == "remove":
                                self.f.reviews.remove(old)
                            else:
                                old["body"] = "Changed old row."
                self.f.read_hook = hook
                self.held("evidence changed before closure")
                self.assertEqual([w[0] for w in self.f.writes], ["POST"])

    def test_review_proof_uses_explicit_edit_metadata(self):
        url = "https://github.com/dev-camo/shadow-decomp/pull/10#pullrequestreview-200"
        proof = {**copy.deepcopy(self.f.comment), "id": 200, "body": "Full independent evidence.",
                 "submitted_at": EARLIER, "commit_id": HEAD, "html_url": url}
        self.f.proof_reviews[10] = [proof]
        self.f.record["evidence"] = url
        self.f.refresh()
        self.assertEqual(self.gate.run(1)["result"], "would close")
        proof["last_edited_at"] = LATER
        self.held("proof changed after acceptance")
        proof["last_edited_at"] = None
        proof["state"] = "DISMISSED"
        self.held("proof dismissed")

    def test_valid_dry_run_never_writes(self):
        result = self.gate.run(1)
        self.assertEqual(result["result"], "would close")
        self.assertEqual({c["sha"] for c in result["evidence"]["checks"]}, {MAIN, MERGE})
        self.assertEqual(self.f.writes, [])

    def test_valid_apply_receipt_then_completed_close_and_duplicate_event(self):
        self.assertEqual(self.gate.run(1, True)["result"], "closed")
        self.assertEqual([w[0] for w in self.f.writes], ["POST", "PATCH"])
        self.assertEqual(self.f.issue["state_reason"], "completed")
        self.assertNotIn("— implementation_author", self.f.writes[0][2]["body"])
        self.held("not open")
        self.assertEqual(len(self.f.writes), 2)

    def test_crash_after_receipt_retries_without_duplicate_comment(self):
        self.f.fail_close = True
        self.held("simulated")
        self.f.fail_close = False
        self.gate.run(1, True)
        self.assertEqual([w[0] for w in self.f.writes], ["POST", "PATCH"])

    def test_retry_after_new_build_attempt_updates_one_receipt(self):
        self.f.fail_close = True
        self.held("simulated")
        self.f.add_build(MERGE, 20)
        self.f.fail_close = False
        self.gate.run(1, True)
        self.assertEqual([w[0] for w in self.f.writes], ["POST", "PATCH", "PATCH"])
        self.assertEqual(sum((c.get("body") or "").startswith(closure.RECEIPT) for c in self.f.comments), 1)

    def test_no_refs_or_thread_resolution_acceptance(self):
        self.f.reviews = [{**self.f.comment, "body": "Refs #1. All threads resolved and CI green."}]
        self.held("marker must begin")

    def test_arbitrary_writer_cannot_grant_acceptance(self):
        self.f.reviews = [{**self.f.comment, "user": {"id": 999, "login": "contributor", "type": "User"}}]
        self.held("verification authority")

    def test_bot_or_login_identity_spoof_cannot_grant_acceptance(self):
        for user in ({**VERIFICATION_AUTHOR, "type": "Bot"}, {**VERIFICATION_AUTHOR, "id": 999}, {**VERIFICATION_AUTHOR, "login": "renamed"}):
            with self.subTest(user=user):
                self.f.reviews = [{**self.f.comment, "user": user}]
                self.held("verification authority")

    def test_permission_removal_invalidates_acceptance(self):
        self.f.permission["permission"] = "read"
        self.held("current write")

    def test_implementation_author_admin_cannot_supply_acceptance_or_review_evidence(self):
        self.f.comment["user"] = IMPLEMENTATION_AUTHOR
        self.held("verification authority")
        self.f.comment["user"] = VERIFICATION_AUTHOR
        self.f.proof["user"] = IMPLEMENTATION_AUTHOR
        self.held("authorized verification author")

    def test_verification_author_cannot_be_fix_implementation_author(self):
        self.f.prs[10]["user"] = VERIFICATION_AUTHOR
        self.held("unauthorized implementation author")

    def test_swapped_authorization_ids_and_logins_are_rejected(self):
        for user in ({**VERIFICATION_AUTHOR, "id": IMPLEMENTATION_AUTHOR["id"]}, {**IMPLEMENTATION_AUTHOR, "id": VERIFICATION_AUTHOR["id"]}):
            with self.subTest(user=user):
                self.f.comment["user"] = user
                self.held("verification authority")

    def test_permission_response_role_identity_must_match(self):
        for field, value in (("id", IMPLEMENTATION_AUTHOR["id"]), ("login", IMPLEMENTATION_AUTHOR["login"]), ("type", "Bot")):
            with self.subTest(field=field):
                self.setUp()
                self.f.permission["user"] = {**VERIFICATION_AUTHOR, field: value}
                self.held("current write")

    def test_implementation_author_current_access_is_checked_without_granting_attester_authority(self):
        self.f.implementation_permission["permission"] = "read"
        self.held("implementation author lacks current write")

    def test_newer_implementation_author_claim_cannot_supersede_independent_verification_acceptance(self):
        self.f.comments.append({**copy.deepcopy(self.f.comment), "id": 101, "user": IMPLEMENTATION_AUTHOR,
                               "created_at": LATER, "updated_at": LATER})
        self.assertEqual(self.gate.run(1)["result"], "would close")

    def test_partial_or_nonindependent_issue_is_held(self):
        for field in ("complete", "independent_review"):
            with self.subTest(field=field):
                self.f.record[field] = False
                self.f.refresh()
                self.held("complete independent")
                self.f.record[field] = True

    def test_later_partial_or_malformed_command_revokes_old_acceptance(self):
        for body in (closure.MARKER + "\n{}", closure.MARKER + "\nnot json"):
            with self.subTest(body=body):
                self.f.reviews = [self.f.comment, {**self.f.comment, "id": 101,
                    "submitted_at": LATER, "created_at": LATER, "updated_at": LATER, "body": body}]
                self.held("invalid acceptance|malformed")

    def test_edit_revokes_old_acceptance(self):
        self.f.comment["last_edited_at"] = LATER
        self.held("edited")




    def test_issue_body_and_main_base_pins(self):
        cases = [("body", "scope changed"), ("title", "scope changed"),
                 ("main", "anchor missing"), ("base", "base mismatch")]
        for kind, reason in cases:
            with self.subTest(kind=kind):
                self.setUp()
                if kind == "body":
                    self.f.issue["body"] += " New acceptance scope."
                elif kind == "title":
                    self.f.issue["title"] += " New unresolved requirement."
                elif kind == "main":
                    self.f.branch["commit"]["sha"] = "0" * 40
                else:
                    self.f.record["base_sha"] = "0" * 40
                    self.f.refresh()
                self.held(reason)

    def test_pr_head_review_base_merge_and_state_are_independent(self):
        for field in ("head", "base", "merge", "merged"):
            with self.subTest(field=field):
                self.setUp()
                pr = self.f.prs[10]
                if field in ("head", "base"):
                    pr[field]["sha"] = "0" * 40
                elif field == "merge":
                    pr["merge_commit_sha"] = HEAD
                else:
                    pr["merged"] = False
                self.held("changed|not merged")

    def test_premerge_acceptance_and_nonancestor_fix_are_held(self):
        self.f.prs[10]["merged_at"] = LATER
        self.held("predates")
        self.f.prs[10]["merged_at"] = EARLIER
        self.f.compare = "diverged"
        self.held("ancestor")

    def test_multiple_fixes_all_need_postmerge_build(self):
        self.f.record["fixes"].append({**self.f.record["fixes"][0], "pr": 11, "merge_sha": SECOND_MERGE})
        self.f.prs[11] = {**copy.deepcopy(self.f.prs[10]), "merge_commit_sha": SECOND_MERGE}
        self.f.checks[SECOND_MERGE], self.f.statuses[SECOND_MERGE], self.f.runs[SECOND_MERGE] = [], [], []
        self.f.refresh()
        self.held("Build missing")
        self.f.add_build(SECOND_MERGE, 3)
        result = self.gate.run(1)
        self.assertEqual(len(result["evidence"]["checks"]), 3)

    def test_duplicate_fixes_rejected(self):
        self.f.record["fixes"].append(copy.deepcopy(self.f.record["fixes"][0]))
        self.f.refresh()
        self.held("duplicate")

    def test_all_region_original_object_source_and_final_dol_proofs(self):
        for kind in ("missing", "object", "source", "dol"):
            with self.subTest(kind=kind):
                self.setUp()
                if kind == "missing":
                    del self.f.record["regions"]["GUPJ8P"]
                else:
                    field = {"object": "objects_exact", "source": "source_linked", "dol": "final_dol_sha1"}[kind]
                    self.f.record["regions"]["GUPJ8P"][field] = False
                self.f.refresh()
                self.held("regional")

    def test_spoofed_missing_or_changed_proof(self):
        self.f.proof["user"] = {"id": 999, "type": "User", "login": "outsider"}
        self.held("authorized verification author")
        self.f.proof["user"] = VERIFICATION_AUTHOR
        self.f.proof["updated_at"] = LATER
        self.held("proof changed")

    def test_external_proof_url_rejected(self):
        self.f.record["evidence"] = "https://attacker.example/run"
        self.f.refresh()
        self.held("local evidence")

    def test_reopen_requires_fresh_acceptance(self):
        self.f.timeline = [{"event": "reopened", "created_at": LATER}]
        self.held("reopened")

    def test_pr_only_success_does_not_count_as_main_build(self):
        self.f.runs[MERGE][0]["event"] = "pull_request"
        self.held("push Build missing")

    def test_latest_build_and_attempt_must_pass_all_jobs(self):
        self.f.add_build(MERGE, 20)
        self.f.runs[MERGE][-1]["conclusion"] = "failure"
        self.held("latest push Build")
        self.f.runs[MERGE][-1].update(conclusion="success", run_attempt=2)
        self.f.jobs[(20, 2)] = copy.deepcopy(self.f.jobs[(20, 1)])
        for job in self.f.jobs[(20, 2)]:
            job["run_attempt"] = 2
        self.f.jobs[(20, 2)][0]["conclusion"] = "failure"
        self.held("unsuccessful jobs")

    def test_partial_rerun_aggregates_latest_execution_per_job_in_same_run(self):
        self.f.jobs[(2, 1)][1]["conclusion"] = "failure"
        self.f.runs[MERGE][0]["run_attempt"] = 2
        retry = copy.deepcopy(self.f.jobs[(2, 1)][1])
        retry.update(id=900, run_attempt=2, conclusion="success")
        self.f.jobs[(2, 2)] = [retry]
        result = self.gate.run(1)["evidence"]
        build = next(c for c in result["checks"] if c["sha"] == MERGE)
        self.assertEqual(build["attempt"], 2)
        self.assertEqual({j["id"] for j in build["jobs"]}, {200, 202, 900})
        retry["conclusion"] = "failure"
        self.held("unsuccessful jobs")

    def test_partial_rerun_cannot_hide_a_missing_region_or_wrong_job_provenance(self):
        self.f.runs[MERGE][0]["run_attempt"] = 2
        self.f.jobs[(2, 1)] = []
        self.f.jobs[(2, 2)] = [{"id": 900, "run_id": 2, "run_attempt": 2, "head_sha": MERGE,
                              "name": "build (GUPJ8P)", "status": "completed", "conclusion": "success"}]
        self.held("missing/ambiguous")
        self.f.jobs[(2, 2)][0]["head_sha"] = HEAD
        self.held("mismatch")

    def test_new_attempt_cannot_qualify_using_only_old_jobs(self):
        self.f.runs[MERGE][0]["run_attempt"] = 2
        self.held("latest Build attempt has no jobs")

    def test_rerun_starting_during_job_reads_invalidates_qualification(self):
        def hook(path):
            if path == self.f.root + "/actions/runs/2/jobs?filter=all":
                self.f.runs[MERGE][0].update(run_attempt=2, status="queued", conclusion=None)
        self.f.read_hook = hook
        self.held("changed during qualification")
        self.assertEqual(self.f.writes, [])

    def test_rerun_attempt_changes_even_after_full_success_hold_before_close(self):
        count = [0]
        def hook(path):
            if path == self.f.root + "/actions/runs/2":
                count[0] += 1
                if count[0] == 4:
                    self.f.runs[MERGE][0]["run_attempt"] = 2
        self.f.read_hook = hook
        self.held("changed during qualification")
        self.assertEqual([w[0] for w in self.f.writes], ["POST"])

    def test_control_main_sha_does_not_substitute_for_upstream_compilation(self):
        with mock.patch.dict(os.environ, {"GITHUB_SHA": MAIN}):
            self.f.runs[MERGE][0].update(event="pull_request", head_sha=HEAD)
            self.held("push Build missing")

    def test_jobs_cannot_come_from_another_run_or_future_attempt(self):
        for field, value in (("run_id", 999), ("run_attempt", 2)):
            with self.subTest(field=field):
                self.setUp()
                self.f.jobs[(2, 1)][0][field] = value
                self.held("mismatch")

    def test_missing_and_duplicate_regional_job_hold(self):
        self.f.jobs[(2, 1)].pop()
        self.held("missing/ambiguous")
        self.f.jobs[(2, 1)].append(copy.deepcopy(self.f.jobs[(2, 1)][0]))
        self.held("missing/ambiguous")

    def required_check(self, sha, identity, state="success", app=15368):
        self.f.checks[sha].append({"id": identity, "head_sha": sha, "name": "Extra", "app": {"id": app},
                                  "status": "completed", "conclusion": state})

    def test_union_classic_and_rules_checks_on_actual_merge_and_main(self):
        self.f.branch["protection"]["required_status_checks"]["contexts"] = ["Legacy"]
        self.f.rules = [{"type": "required_status_checks", "parameters": {
            "required_status_checks": [{"context": "Extra", "integration_id": 15368}]}}]
        for sha in (MAIN, MERGE):
            self.f.statuses[sha] = [{"id": 1, "context": "Legacy", "state": "success"}]
            self.required_check(sha, 1)
        self.assertEqual(self.gate.run(1)["result"], "would close")
        self.required_check(MERGE, 2, state="failure")
        self.held("not successful")

    def test_latest_pending_status_and_wrong_app_block(self):
        self.f.branch["protection"]["required_status_checks"]["contexts"] = ["Legacy"]
        self.f.statuses[MAIN] = [{"id": 1, "context": "Legacy", "state": "success"},
                                 {"id": 2, "context": "Legacy", "state": "pending"}]
        self.held("not successful")
        self.f.branch["protection"]["required_status_checks"] = {
            "contexts": ["Extra"], "checks": [{"context": "Extra", "app_id": 15368}]}
        self.required_check(MAIN, 1, app=999)
        self.held("missing")

    def test_missing_policy_and_required_workflow_rule_hold(self):
        self.f.rules = [{"type": "workflows"}]
        self.held("unsupported")
        self.f.rules = []
        self.f.branch.pop("protection")
        self.held("unavailable")

    def test_failed_skipped_neutral_cancelled_and_pending_required_checks_hold(self):
        self.f.branch["protection"]["required_status_checks"]["checks"] = [{"context": "Extra", "app_id": 15368}]
        for n, result in enumerate(("failure", "skipped", "neutral", "cancelled", "timed_out", None), 1):
            with self.subTest(result=result):
                self.required_check(MAIN, n, state=result)
                self.held("not successful")

    def test_duplicate_success_from_other_app_does_not_override_failure(self):
        self.f.branch["protection"]["required_status_checks"]["contexts"] = ["Extra"]
        self.required_check(MAIN, 1, state="failure")
        self.required_check(MAIN, 2, state="success", app=999)
        self.held("not successful")

    def test_changed_default_branch_and_cross_repo_fix_hold(self):
        self.f.repo["default_branch"] = "release"
        self.held("default branch changed")
        self.f.repo["default_branch"] = "main"
        self.f.prs[10]["base"]["repo"]["id"] = 999
        self.held("not merged")

    def test_dismissed_review_evidence_holds(self):
        value = "https://github.com/dev-camo/shadow-decomp/pull/10#pullrequestreview-200"
        self.f.record["evidence"] = value
        self.f.proof_reviews[10] = [{**copy.deepcopy(self.f.comment), "id": 200,
            "html_url": value, "state": "DISMISSED", "submitted_at": EARLIER}]
        self.f.refresh()
        self.held("dismissed")

    def test_state_race_before_receipt_and_before_close(self):
        for reads in (2, 3):
            with self.subTest(reads=reads):
                self.setUp()
                count = [0]
                def hook(path):
                    if path == self.f.root + "/issues/1":
                        count[0] += 1
                        if count[0] == reads:
                            self.f.issue["body"] = "Changed scope"
                self.f.read_hook = hook
                self.held("scope changed")
                self.assertEqual(len(self.f.writes), 0 if reads == 2 else 1)

    def test_title_change_before_each_full_validation_holds(self):
        for reads in (1, 2, 3):
            with self.subTest(reads=reads):
                self.setUp()
                count = [0]
                def hook(path):
                    if path == self.f.root + "/issues/1":
                        count[0] += 1
                        if count[0] == reads:
                            self.f.issue["title"] = "New unresolved requirement"
                self.f.read_hook = hook
                self.held("scope changed")
                self.assertEqual(len(self.f.writes), 1 if reads == 3 else 0)












    def test_authorization_configuration_rejects_overlap_and_invalid_logins(self):
        invalid = copy.deepcopy(self.f.authorization)
        invalid["implementation_authors"] = copy.deepcopy(invalid["verification_authors"])
        with self.assertRaisesRegex(closure.Hold, "must be separate"):
            closure.Gate(self.f, invalid)
        invalid["implementation_authors"] = [{"id": 10101, "login": "bad/name"}]
        with self.assertRaisesRegex(closure.Hold, "invalid authorization identity"):
            closure.Gate(self.f, invalid)

    def test_proof_change_race_is_held(self):
        count = [0]
        def hook(path):
            if path == self.f.root + "/issues/1":
                count[0] += 1
                if count[0] == 2:
                    self.f.proof["body"] += " changed"
        self.f.read_hook = hook
        self.held("changed before receipt")

    def test_copied_receipt_cannot_replace_bot_receipt(self):
        snapshot = self.gate.snapshot(1)
        self.f.comments.append({"id": 500, "user": {**VERIFICATION_AUTHOR, "id": 999}, "body": self.gate.receipt_body(snapshot),
                                "created_at": TIME, "updated_at": TIME})
        self.gate.run(1, True)
        self.assertEqual([w[0] for w in self.f.writes], ["POST", "PATCH"])

    def test_json_duplicate_keys_and_extraneous_commands_are_rejected(self):
        self.f.comment["body"] = closure.MARKER + '\n{"schema":1,"schema":1}'
        self.held("duplicate")
        self.f.record["command"] = "execute me"
        self.f.refresh()
        self.held("invalid acceptance fields")


class PaginationTests(unittest.TestCase):
    def test_graphql_history_reads_every_page_and_large_review_ids(self):
        fixture = Fixture()
        sample = fixture.request("POST", "/graphql", {"query": closure.REVIEW_QUERY, "variables": {"number": 20}})
        node = sample["data"]["repository"]["pullRequest"]["reviews"]["nodes"][0]
        calls = []
        def request(method, path, body=None):
            self.assertEqual((method, path, body["query"]), ("POST", "/graphql", closure.REVIEW_QUERY))
            calls.append(body["variables"]["cursor"])
            response = copy.deepcopy(sample)
            connection = response["data"]["repository"]["pullRequest"]["reviews"]
            connection["totalCount"] = 101
            if len(calls) == 1:
                connection["nodes"] = [{**node, "fullDatabaseId": str(5_000_000_000 + i)} for i in range(100)]
                connection["pageInfo"] = {"hasNextPage": True, "endCursor": "page-one"}
            else:
                connection["nodes"] = [{**node, "fullDatabaseId": "5000000100"}]
            return response
        fixture.request = request
        rows = closure.Gate(fixture, fixture.authorization).reviews(20, "9" * 40)
        self.assertEqual(len(rows), 101)
        self.assertEqual(rows[-1]["id"], 5_000_000_100)
        self.assertEqual(calls, [None, "page-one"])

    def test_graphql_duplicate_rows_cursor_cycles_and_exhaustion_hold(self):
        for defect in ("duplicate", "cycle", "overflow"):
            with self.subTest(defect=defect):
                fixture = Fixture()
                sample = fixture.request("POST", "/graphql", {"query": closure.REVIEW_QUERY, "variables": {"number": 20}})
                calls = [0]
                def request(method, path, body=None):
                    calls[0] += 1
                    response = copy.deepcopy(sample)
                    connection = response["data"]["repository"]["pullRequest"]["reviews"]
                    if defect == "duplicate":
                        connection["nodes"] *= 2
                        connection["totalCount"] = 2
                    else:
                        connection["pageInfo"] = {"hasNextPage": True,
                            "endCursor": "same" if defect == "cycle" else str(calls[0])}
                    return response
                fixture.request = request
                with self.assertRaisesRegex(closure.Hold, "duplicate|pagination"):
                    closure.Gate(fixture, fixture.authorization).reviews(20)
                self.assertLessEqual(calls[0], 50)

    def test_graphql_mutation_or_changed_operation_is_rejected_before_network(self):
        api = closure.Api("unused")
        with mock.patch.object(closure.urllib.request, "build_opener") as opener:
            for method, body in (("POST", {"query": "mutation { closeIssue(input: {}) { clientMutationId } }", "variables": {}}),
                                 ("GET", {"query": closure.REVIEW_QUERY, "variables": {}}),
                                 ("POST", {"query": closure.REVIEW_QUERY, "variables": {}, "operationName": "Other"})):
                with self.assertRaisesRegex(closure.Hold, "fixed review-history"):
                    api.request(method, "/graphql", body)
            opener.assert_not_called()

    def test_api_failure_does_not_publish_private_lookup_identity(self):
        api = closure.Api("unused")
        opener = mock.Mock()
        opener.open.side_effect = urllib.error.URLError("private-identity")
        with mock.patch.object(closure.urllib.request, "build_opener", return_value=opener):
            with self.assertRaises(closure.Hold) as failure:
                api.get("/fixture/private-identity")
        self.assertEqual(str(failure.exception), "GitHub API request failed: GET")

    def test_reads_multiple_pages_and_caps_exhaustion(self):
        api = closure.Api("unused")
        calls = []
        def get(path):
            calls.append(path)
            page = urllib.parse.parse_qs(urllib.parse.urlsplit(path).query)["page"][0]
            return list(range(100)) if page == "1" else [100]
        api.get = get
        self.assertEqual(len(api.list("/fixture")), 101)
        self.assertEqual(len(calls), 2)
        api.get = lambda _: list(range(100))
        with self.assertRaisesRegex(closure.Hold, "pagination limit"):
            api.list("/fixture")

    def test_truncated_check_collection_holds(self):
        api = closure.Api("unused")
        api.get = lambda _: {"total_count": 101, "check_runs": []}
        with self.assertRaisesRegex(closure.Hold, "truncated"):
            api.list("/fixture", "check_runs")


class ExecutionGuardTests(unittest.TestCase):
    def test_review_listener_has_no_checkout_token_or_protected_configuration(self):
        root = pathlib.Path(__file__).resolve().parents[1]
        listener = (root / "workflows/issue-acceptance-event.yml").read_text()
        self.assertIn("permissions: {}", listener)
        self.assertIn("run: 'true'", listener)
        self.assertNotIn("uses:", listener)
        self.assertNotIn("secrets.", listener)
        self.assertNotIn("github.token", listener)
        controller = (root / "workflows/verified-issue-closure.yml").read_text()
        self.assertNotIn("issue_comment:", controller)
        self.assertIn("ref: ${{ github.sha }}", controller)
        self.assertIn("github.event.workflow_run.event == 'pull_request_review'", controller)

    def test_dispatch_job_guard_precedes_checkout_and_branch_execution_fails_before_api(self):
        root = pathlib.Path(__file__).resolve().parents[1]
        workflow = (root / "workflows/verified-issue-closure.yml").read_text()
        guard = workflow.split("runs-on:", 1)[0]
        self.assertIn("(github.event_name == 'workflow_dispatch' && github.ref == 'refs/heads/main')", guard)
        env = {**os.environ, "GITHUB_REPOSITORY": closure.REPOSITORY, "GITHUB_REF": "refs/heads/untrusted",
               "GITHUB_EVENT_NAME": "workflow_dispatch", "VERIFIED_ISSUE_CLOSURE_ENABLED": "true"}
        env.pop("GITHUB_TOKEN", None)
        result = subprocess.run([sys.executable, "-B", str(root / "scripts/verified_issue_closure.py"), "--apply"],
                                env=env, capture_output=True, text=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("trusted main execution required", result.stderr)
        self.assertNotIn("GITHUB_TOKEN", result.stderr)


if __name__ == "__main__":
    unittest.main()
