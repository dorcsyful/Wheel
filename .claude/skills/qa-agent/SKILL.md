---
name: qa-agent
description: Closed-loop mutation-guided test generation for the Wheel engine's deterministic sim core (math/collision/physics/constraint solver). Reads coverage and mutation gaps, writes one new test against the oracle API, submits it through qa/agent_interface, and iterates. Invoke with /qa-agent [step-budget].
---

# QA agent loop

You are extending the hand-written test suite for the Wheel engine's
deterministic sim core, guided by real adequacy signal (coverage delta +
mutation survival) rather than guessing. The harness, oracle library,
coverage, mutation testing, and tool interface already exist and are proven
working — you are the first consumer of `qa/agent_interface/submit_test.py`,
not building it.

## Mission (non-negotiable)

Your job is to **fight the engine, not conform to it**. You are trying to
*break* the code — with egregious/extreme values and ordinary ones alike — and
a test that fails because it exposed a real defect is a SUCCESS.

- **Never delete, disable (`DISABLED_`, `GTEST_SKIP`), or weaken a test to make
  a run pass.** If a test you wrote fails and you believe it's asserting correct
  behaviour, LEAVE IT FAILING and flag the bug (file:line + the values/trajectory
  that break it). A red suite caused by a real defect is the correct state.
- **Assert what is correct** (the oracle / physics ground truth) — never conform
  an assertion to the engine's current (possibly buggy) output. An oracle you
  bend to match the engine is pointless.
- **Never edit `wheel/` to make a test pass.** Flag suspected engine bugs; don't
  fix them from this loop.
- **All-green is not the goal; exposing defects is.** If green comes easily,
  assume you forgot to check something or conformed to the engine — not that the
  code is perfect. A red baseline correctly blocks the mutation green-baseline
  gate; that's the tooling refusing to score a broken engine, not a reason to
  soften tests.

Read both reference files before writing anything:
- `references/oracle_api.md` — the 9 reusable assertions you must build
  tests on top of. Every test needs a positive case *and* a negative/
  sensitivity case proving the oracle would actually catch a wrong value.
- `references/engine_api.md` — how to build a scene, and the load-bearing
  cautions that will silently produce a meaningless test if you miss them
  (no-collider bodies don't get simulated as expected, gravity is per-scene
  and settable via SetGravity/GetGravity so don't hardcode it, non-zero
  default damping/restitution, and the rule that a joint enforces only what
  its solver actually does — read the solver, don't trust the flag names).
- `references/edge_case_catalogue.md` — where to aim: a durable map of the
  solver regions and physical scenarios most worth probing. Use it to choose
  a region; use the live `coverage_delta`/`mutation` output from
  `submit_test.py` (not this file) as the source of truth for what is still
  uncovered on the run you're doing.

## Environment

Everything here runs from WSL/Linux (gcov, the coverage/mutation tooling —
none of it works from a plain Windows shell). From a Windows Claude Code
session, invoke it like this:

```
wsl.exe -d Ubuntu -- bash -lc "cd /mnt/c/Users/dorcs/Desktop/Wheel/qa/agent_interface && ./run_agent_interface.sh --file <Name>.cpp --content-file <path> [--with-mutation]"
```

`qa/agent_tests/` is the *only* directory you write to. `submit_test.py`
enforces this (bare filename, no path traversal, refuses to overwrite an
existing file without `--overwrite`) — you don't additionally need to be
careful about it, but don't try to work around it either. Never edit
`wheel/`, `qa/harness/`, or `qa/oracles/` from this loop; if a gap in the
*oracle library itself* seems needed (a 10th assertion), stop and say so
instead of adding it yourself.

If `qa/agent_interface/baseline_coverage.json` looks stale (the hand-written
suite changed since it was captured), regenerate it first:
`./run_agent_interface.sh --capture-baseline`.

## The loop

Default step budget is 5 if `/qa-agent` was invoked with no argument; use
the given number otherwise. One step = one test file submitted (whether
accepted, revised, or flagged).

For each step:

1. **Pick one target**: choose a region from the edge-case catalogue, then
   let the current `coverage_delta`/`mutation` output confirm whether it's
   actually still under-covered before you invest in it. Don't try to cover
   multiple unrelated things in one test — one scenario, one or two
   closely-related oracle assertions.

2. **Read the relevant engine source** for that target (e.g. if targeting
   the `Solve2ContactConstraint` branch-selection gap, read
   `wheel/physics/CollisionConstraintSolver.cpp` around it) so the test's
   assertions are grounded in what the code actually does, not a guess.

3. **Write one test file** in a scratch location, using the oracle API and
   engine API per the references. Follow the existing `qa/oracle_tests/*.cpp`
   style: build a scene, run it, compute the oracle inputs, assert, plus a
   companion test that calls the oracle directly with a wrong value to prove
   it discriminates.

4. **Submit it**: `--file <DescriptiveName>.cpp --content-file <scratch path>`.
   Use `--with-mutation` at least every other step — it's the slow path but
   it's also the actual point (a test that adds coverage without killing
   mutants isn't done yet).

5. **Interpret the structured result**:
   - `build.success == false` → your test doesn't compile. Fix it and
     resubmit with `--overwrite`; don't count this as a wasted step if you
     catch it within the same turn.
   - A test **you wrote** fails (`tests.failures` includes your new test
     name) → read the failure message. Decide: is your test's expectation
     wrong (fix and resubmit), or did it just catch a real engine bug? If
     you genuinely believe it's a real bug, **stop and flag it explicitly**
     in your final report with the file:line and reasoning — do not "fix"
     the engine yourself from this loop, and do not delete or weaken the
     test to make it pass. (Step 8 will formalize this accept/flag gate;
     until then, use judgment and always err toward flagging rather than
     silently accepting or silently discarding.)
   - `coverage_delta.newly_covered_lines` near zero → this test didn't add
     anything the suite doesn't already exercise. Pick a different target
     next step rather than iterating on this one.
   - `mutation.survived > 0` (when you ran with `--with-mutation`) → your
     test covers the code but its assertions aren't sensitive enough to
     notice the listed mutations. Look at each surviving mutant's
     `description` and strengthen the test to distinguish that specific
     case (e.g. assert on an intermediate velocity/impulse, not just a
     final position) — this is usually more valuable than moving on to a
     new target.

6. **Record what you found in your run report, not in the reference files.**
   The per-run signal — which mutants survived, which lines you newly covered,
   which scenario you didn't get to — belongs in the final report (and in the
   tool's own output), which is where the next run reads live state from. Only
   edit `references/edge_case_catalogue.md` if you discover a *durable* new
   error-prone region worth adding to the map — not to log this run's status.

## Termination

Stop before the step budget is exhausted if:
- **Mutation-score plateau**: two consecutive `--with-mutation` submissions
  both show `survived == new_mutants` (nothing you're writing is killing
  anything) — the targets you're picking aren't working, not that the
  engine is fully tested. Reconsider the target list rather than continuing.
- **Coverage plateau**: two consecutive submissions with
  `newly_covered_lines == 0`.
- Otherwise, stop at the step budget.

## Final report

End with: how many tests submitted / accepted, total newly covered lines
this run, mutation kill rate on what you tested (killed / (killed+survived)),
and an explicit list of anything flagged for human review (real suspected
engine bugs, oracle-library gaps you didn't have authority to fix). Don't
bury a flagged bug in a wall of accepted-test summary — lead with it.