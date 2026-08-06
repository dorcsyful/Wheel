# Oracle API reference

Header: `qa/oracles/Oracles.h` · Namespace: `Wheel::QA` · Link target: `qa_oracles`
(pulls in `qa_harness` and `wheel_engine` transitively).

Every oracle returns `OracleResult { bool passed; std::string message; }` —
framework-agnostic, not a gtest type. Wrap with:

```cpp
QA::OracleResult result = QA::SomeOracle(...);
EXPECT_TRUE(result.passed) << result.message;
```

Every new test should have a **negative/sensitivity companion**: call the
oracle directly with a hand-crafted wrong value and assert `!result.passed`.
A test that only ever proves an oracle *passes* doesn't prove the oracle
would catch anything — see `qa/oracle_tests/*.cpp` for the pattern (every
oracle there has a `*_OracleCatches*` test alongside its happy-path test).

## Analytical

```cpp
float KineticEnergy(float mass, const Math::Vector2& linearVelocity,
                     float inertia = 0.0f, float angularVelocity = 0.0f);

OracleResult AnalyticalFreeFallMatches(float actualY, float y0, float v0y,
                                        float gravityY, float deltaTime, int numSteps,
                                        float tolerance);
```
Ground truth is the **discrete** semi-implicit-Euler closed form
(`x0 + n*v0*dt + g*dt^2*n*(n+1)/2`), not continuous ½gt² — the engine
integrates `v += a*dt; x += v*dt` each step, so comparing to the continuous
formula would fail for the wrong reason. `gravityY` must match whatever the
scene's `Physics2DSystem` actually uses. Gravity is per-scene and settable
via `SetGravity()`/readable via `GetGravity()` — see engine_api.md — so pass
the value the test itself set, or read it back with `GetGravity()`. Never
hardcode a gravity magnitude the test didn't set and read.

```cpp
OracleResult ElasticEqualMassCollisionSwapsVelocity(
    const Math::Vector2& velocityABefore, const Math::Vector2& velocityBBefore,
    const Math::Vector2& velocityAAfter, const Math::Vector2& velocityBAfter,
    float tolerance);
```
Equal mass, restitution 1, head-on: velocities should exchange.

## Invariants

```cpp
OracleResult NoPenetrationPastSlop(const Collision::Collision2DManifold& manifold,
                                    float slop = 0.01f, float tolerance = 1e-3f);
```
Pass a manifold **freshly recomputed from final transforms** (e.g. via
`BoxBoxCollision2D::BoxBoxCollision` / `CircleBoxCollision2D::CheckCircleBoxCollision` /
`CircleCircleCollision2D::CheckCircleCircleCollision`), not one cached
mid-solve. `slop` should match the engine's actual `SLOP` constant — read its
current value from `CollisionConstraintSolver.cpp` and pass that — unless the
test is deliberately probing that constant.

```cpp
OracleResult MomentumConserved(const std::vector<float>& masses,
                                const std::vector<Math::Vector2>& velocitiesBefore,
                                const std::vector<Math::Vector2>& velocitiesAfter,
                                float tolerance);
```
Only meaningful with **no external force**: set `affectedByGravity = false`
on every body, no joints. Contact impulses are equal-and-opposite by
construction so momentum should hold even through a collision.

```cpp
OracleResult EnergyNonIncreasing(float energyBefore, float energyAfter, float tolerance);
```
Use `KineticEnergy()` for both sides. Only holds when every restitution
involved is `< 1`.

```cpp
OracleResult RestingBodyStaysAtRest(const std::vector<Math::Vector2>& linearVelocitySamples,
                                     const std::vector<float>& angularVelocitySamples,
                                     float velocityEpsilon);
```
Settle first (many steps), *then* sample: call `QA::RunScene(scene, 1)`
repeatedly in a loop, collecting the velocity from each returned snapshot.

```cpp
OracleResult DistanceJointSatisfied(const Math::Vector2& anchor1, const Math::Vector2& anchor2,
                                     float targetDistance, bool maxDistanceOnly, float tolerance);
```
Call this oracle with the `maxDistanceOnly` argument that matches the
constraint the solver actually enforces, which is not necessarily the same as
the joint component's flag. **Read engine_api.md's joint caution and the
current `JointConstraintSolver.cpp` before using this**: derive the expected
constraint (rope-like max-distance, rigid rod, etc.) from what that solver
does for your flag values, then pass the corresponding `maxDistanceOnly` here.
A test written against the solver's actual behaviour stays correct across
changes to what the flag means.

## Metamorphic

```cpp
OracleResult MirroredScenesMatch(const std::vector<EntitySnapshot>& original,
                                  const std::vector<EntitySnapshot>& mirroredX,
                                  float tolerance);
```
Build two scenes: one normal, one with every initial `x`, `vx`, `rotation`,
`angularVelocity` negated. Gravity acts along Y only, so Y motion is
untouched by the mirror. Both `RunScene` results are sorted by entity id
already (see `HeadlessSim.h`), so they line up index-for-index.

## Golden-master

```cpp
OracleResult MatchesGoldenMaster(const std::vector<EntitySnapshot>& actual,
                                  const std::string& goldenFilePath, bool recordIfMissing = true);
```
Bit-for-bit via raw IEEE-754 hex bit patterns (not `std::hexfloat` text —
GCC 13's libstdc++ misparses negative hex-float literals, see
`qa/oracles/Oracles.cpp` history). First call with no existing file records
a baseline and returns `passed=false` ("recorded, re-run to compare") —
that's the correct bootstrap behavior, not a bug. Existing baselines live
in `qa/oracle_tests/golden/`; a new golden-master test should put its file
there too and get committed once verified stable.