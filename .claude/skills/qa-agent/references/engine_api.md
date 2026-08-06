# Engine API reference (for writing agent_tests)

## Building a scene

```cpp
#include "HeadlessSim.h"
#include "core/Scene.h"
#include "common/Transform2D.h"
#include "collision/Collider2D.h"
#include "physics/Rigidbody2D.h"
#include "physics/Joint2D.h"          // only if using DistanceJoint2D

Core::Scene* scene = QA::CreateHeadlessScene();
// registers Transform2D/BoxCollider2D/CircleCollider2D/Rigidbody2D/DistanceJoint2D
// and Collision2DSystem -> Physics2DSystem in that order. No renderer, no window.

uint32_t id = scene->AddEntity();                                   // sequential from 0 in a fresh scene
scene->AddComponent<Common::Transform2D>(id).SetPosition(x, y);
scene->AddComponent<Collision::CircleCollider2D>(id).radius = 0.5f; // or BoxCollider2D
Physics::Rigidbody2D& rb = scene->AddComponent<Physics::Rigidbody2D>(id);
rb.SetType(Physics::Rigidbody2DType::DYNAMIC);  // or STATIC / KINEMATIC
rb.SetMass(1.0f);

auto snapshots = QA::RunScene(*scene, numSteps /*, fixedDeltaTime = 1/60 */);
// vector<EntitySnapshot{entityId, position, rotation, linearVelocity, angularVelocity}>
// sorted by entityId, deterministic (fixed dt, never wall-clock).

delete scene; // caller owns it
```

To read intermediate state instead of just the final snapshot, call
`QA::RunScene(*scene, 1)` repeatedly and read from the returned vector each
time, or reach into components directly: `scene->GetComponent<Physics::Rigidbody2D>(id)`.

## How to keep a test meaningful

The engine's *behaviour* — solver constants, component defaults, which
constraints a joint actually enforces, whether a code path is a real feature
or an unfixed gap — is not fixed by this document and will drift as the engine
changes. This reference gives you the stable API surface and a set of durable
cautions. **Whenever a test depends on a specific numeric constant or on the
precise behaviour of a code path, read the current value or logic from the
engine source and derive the test's expectation from that**, rather than
copying a number or a behavioural claim out of any doc. Tests written that way
keep passing across bug fixes and feature work; tests that hardcode today's
incidental behaviour break on the next change.

## Load-bearing cautions

These are the traps that silently produce a test that compiles, passes, and
proves nothing. They are properties of how a headless physics scene is set up,
not statements about any particular engine version's tuning.

- **A dynamic `Rigidbody2D` needs a collider to be simulated as expected.**
  The per-body force/velocity step in `Physics2DSystem::Update` is tied to
  inertia computation, which needs a collider; a dynamic body without one can
  be skipped entirely (no gravity, no forces). Always give a dynamic body a
  `BoxCollider2D` or `CircleCollider2D`, even in a scene where it never
  actually touches anything. Confirm the current coupling in
  `wheel/physics/Physics2DSystem.cpp` if a bodiless-collider test ever
  behaves unexpectedly.

- **Gravity is per-scene and settable, not a global constant.**
  `Physics2DSystem::SetGravity(const Math::Vector2&)` /
  `GetGravity()`. An analytical test must use whatever value the scene's
  system actually holds: either call `SetGravity()` explicitly at the top of
  the test and reuse that exact value in the oracle call, or read it back with
  `GetGravity()`. Never hardcode a gravity magnitude that the test didn't
  itself set and read.

- **`Rigidbody2D` defaults are not "clean."** A freshly added rigidbody
  carries nonzero default restitution and damping (see `wheel/physics/Rigidbody2D.h`
  for the current values). For an analytical test that assumes frictionless,
  undamped, or perfectly (in)elastic behaviour, set every property the test
  depends on explicitly — don't inherit a default — unless the test is
  specifically *about* the default value, in which case read that value from
  the header rather than transcribing it here.

- **A joint enforces whatever its solver currently does, which may be less
  than its field names imply.** `DistanceJoint2D` exposes flags such as
  `maxDistanceOnly`, but the constraint the solver actually applies is defined
  in `wheel/physics/JointConstraintSolver.cpp`. Before asserting on jointed
  motion, read that solver and write the test against the constraint it
  actually enforces for the flag values you set — do not assume a flag's name
  or doc comment describes its effect. This keeps the test correct both before
  and after any change to what the flag does.

- **Solver constants live in source, not here.** Penetration slop, the maximum
  positional correction (both in `wheel/physics/CollisionConstraintSolver.cpp`)
  and the constraint-iteration count (`MAX_CONSTRAINT_ITERATION` in
  `wheel/core/Globals.h`) are compile-time constants. When a test's expectation
  depends on one — e.g. an allowed penetration tolerance — read the current
  value from source and pass it into the oracle, rather than embedding a
  literal that a retune would silently invalidate.

- **Entity ids are reused.** `EntityManager` hands out the lowest free id, so a
  fresh `Scene` always starts at `0, 1, 2, ...`. Don't assume ids survive
  entity removal.

- **Collision narrow-phase is brute-force O(n²).** There is no broad-phase
  acceleration in the headless path. This is fine for the small scenes these
  tests use; just don't build a test that depends on hundreds of bodies for
  timing reasons.

## Component registration

`CreateHeadlessScene()` already registers everything a test needs
(`Transform2D`, `BoxCollider2D`, `CircleCollider2D`, `Rigidbody2D`,
`DistanceJoint2D`) and both systems (`Collision2DSystem` before
`Physics2DSystem` — order matters, physics reads this frame's manifolds out
of collision). Don't call `RegisterComponentType`/`RegisterSystem` yourself
unless deliberately testing something outside that default setup.