# Physics edge-case catalogue

Where to aim a new test. This is a durable map of the sim core's error-prone
regions and the physical scenarios that stress them — not a log of which
mutants happen to survive right now. Treat each entry as a class of failure
worth probing; the authoritative, up-to-date signal for *what is still
uncovered* is the coverage delta and mutation survivors reported by
`submit_test.py` on the run you are doing, not this file. Pick a region below,
read the relevant solver source, then let the tool's output tell you whether
your test actually bit.

## Constraint-solver regions

The contact solver is the densest source of subtle bugs. Each item names a
region and the scenario that exercises it in a way a weak test would miss.

- **Two-contact box resolution (`Solve2ContactConstraint` / the
  `contactCount == 2` path in the constraint resolver).** A box landing flat
  or on an offset centre of mass produces two simultaneous contacts solved
  together via a 2×2 system. The discriminating scenarios are the ones that
  drive the solver's *case selection* — a box tipping far enough that one
  corner's impulse target goes negative and is clamped to zero *during the
  landing transient*, not merely a body that rests stably asymmetric. Assert
  on the transient (an intermediate angular velocity or impulse), not only the
  settled pose, or the case branches go untested.

- **Singular / near-singular contact geometry.** Coincident or near-coincident
  contact points make the 2×2 effective-mass matrix singular; the solver has a
  determinant fallback (`Matrix2x2::Inverse` and the corresponding guard in
  `CollisionConstraintSolver.cpp`). To exercise the fallback *itself* rather
  than just the healthy path, construct geometry that forces two contact points
  to (nearly) the same location and check the resolved state stays sane.

- **Restitution-bias gating.** The branch that decides when a restitution bias
  is applied (velocity-threshold gates in `CollisionConstraintSolver.cpp`) is
  easy to cover but hard to *discriminate*: a test must distinguish "bias
  applied" from "bias skipped." Probe near the threshold — an impact just fast
  enough vs. just slow enough to bounce.

- **Static-body impulse paths (`invMass`/`invInertia` guards).** A STATIC body
  has `invMass == 0` exactly, so some guards around it are genuinely
  unreachable in a meaningful way (a mutant on such a guard can be equivalent).
  Don't burn a step trying to kill a guard that a static body's `0` mass makes
  a no-op; confirm reachability against real dynamic-vs-static contact first.

- **Warm-start impulse matching.** The resolver matches cached impulses from
  the previous frame to this frame's contacts by feature id
  (`Constraint2DResolver.cpp`). A test that runs enough steps for warm-starting
  to matter, then perturbs which contacts persist frame-to-frame, exercises the
  matching logic that a single-step test never reaches.

- **Iteration budget and per-iteration gating.** `MAX_CONSTRAINT_ITERATION`
  bounds the solve, and some work (e.g. friction) is gated to a specific
  iteration index (`i + 1 == MAX_CONSTRAINT_ITERATION`). A stack tall enough
  that convergence actually needs the full budget, sampled from a standing
  start, is what makes iteration-count sensitivity observable.
  Note that a target already reachable by the hand-written suite may not appear
  in a delta-scoped mutation submission at all — verify iteration-budget
  sensitivity against the full mutation sweep, not only the loop interface.

- **Contact-array loop bounds.** Off-by-one on the contact loops
  (`i < contactCount` vs `<=`) recurs across the solver files. Tests that
  assert on *every* contact's outcome in a multi-contact manifold, not just the
  aggregate body motion, are what catch these.

## Narrow-phase collision regions

The SAT box-box and circle-box routines (`BoxBoxCollision2D.cpp`,
`CircleBoxCollision2D.cpp`) are exercised by the hand-written suite but tend to
be asserted only on the *fact* of collision, not on the exact manifold they
produce — so their internal decisions are the least-pinned logic in the sim
core. The discriminating move throughout this area is to assert on the full
manifold (normal direction, penetration depth, contact points, contact count),
not just a boolean "did they collide."

- **Separating-axis gap test.** The per-axis overlap check that decides "these
  shapes are disjoint" (`min1 >= max2 || min2 >= max1`). The boundary variants
  (`>=` vs `>`, and the disjunction itself) only show up in a test that puts two
  boxes *exactly* edge-to-edge (touching, zero penetration) and one hair apart,
  and asserts collision vs. no-collision across that line.

- **Minimum-translation-axis selection.** Picking the least-penetration axis as
  the collision normal (`overlap < minOverlap`, and the cross-collider
  `minOverlap1 < minOverlap2` tiebreak). A box overlapping another where the x-
  and y-penetrations are close — and a case where they're deliberately equal —
  forces the selection and the tiebreak to be correct, which a deep symmetric
  overlap never does.

- **Collision-normal orientation.** The sign checks that flip the chosen axis so
  the normal consistently points A→B (`direction.Dot(normal) < 0`, the
  `cachedNormals[bestAxis].Dot(...)` checks). Assert the normal's *direction*,
  not just its axis, with the two bodies approached from each side.

- **Incident-face selection and clipping.** Choosing the most anti-parallel
  face (`dot < lowestDot`) and the Sutherland–Hodgman clip that keeps or
  discards candidate contact points by signed distance to the reference face
  (`distance <= 0`, the clip tolerance). These decide contact *count* and
  *position*; only a test asserting the actual contact points (and that a
  face-face contact yields two, a corner one) pins them.

- **Circle-vs-box region tests.** The axis pick for a circle against a box face
  (`xPen < yPen`) and the corner-region containment test
  (`distanceSquared > radius * radius`) deciding face contact vs. corner
  contact. A circle placed against a face, and one tucked against a corner
  within/without its radius, separate these branches.

## Physical scenarios worth systematic coverage

- **Tunneling.** A small, fast body crossing a thin static collider within one
  fixed step. There is no continuous collision detection, so this is expected
  to be a real limitation — a good test *documents* the failure mode explicitly
  rather than silently assuming it can't happen.

- **Extreme mass ratios** (1:1000 and beyond). Does the heavy body stay put
  under a light one, or does the solver let the light body shove it
  unrealistically?

- **Degenerate manifolds.** Two colliders producing coincident (zero-width)
  contact — overlaps the singular-geometry solver region above; approach it
  from the manifold-generation side too.

- **Perfectly elastic (`restitution = 1`) beyond the equal-mass circle case.**
  Unequal masses, box-box, box-circle. Momentum and the velocity-exchange
  relations should still hold.

- **Long-horizon resting stability.** A settled stack sampled over hundreds or
  thousands of steps: does it stay asleep, or does numerical drift wake it? The
  resting-body oracle exists, but tends to be used over short windows — extend
  the horizon.

- **Joint + collision interaction.** A jointed body that is simultaneously in
  contact with something else: do the two constraint types fight, converge, or
  destabilise each other?

## Working with the tooling

- `coverage_delta` and `mutation` are computed against the fixed
  `baseline_coverage.json` (the hand-written suite), not against other
  `agent_tests/*.cpp`. Consequences to plan around:
  - A region already reachable by the hand-written suite will not be
    mutation-scored through `--with-mutation`, no matter how good the new test
    is. Behavioural regression tests for such regions are still worth writing;
    just don't expect a mutation number to grade them, and verify against the
    full sweep (`qa/mutation/run_mutation.sh`) when you need real signal.
  - Once an agent test opens a new delta line, that line stays in every
    subsequent submission's delta. The way to close a stubborn surviving mutant
    is to *strengthen the test that first opened that region* with sharper
    assertions, not to add an unrelated new test.