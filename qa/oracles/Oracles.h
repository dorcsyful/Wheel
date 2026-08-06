#pragma once
#include <string>
#include <vector>

#include "math/Vector2.h"
#include "collision/Collision2DManifold.h"
#include "HeadlessSim.h"

namespace Wheel
{
    namespace QA
    {
        /**
         * @brief Framework-agnostic pass/fail verdict. Deliberately not a gtest
         * type: oracles are called from hand-written gtest tests today, but the
         * agent loop (step 5+) needs the same verdict shape without depending on
         * a test framework.
         */
        struct OracleResult
        {
            bool passed = true;
            std::string message; // empty when passed

            static OracleResult Pass() { return OracleResult{ true, "" }; }
            static OracleResult Fail(std::string a_Message) { return OracleResult{ false, std::move(a_Message) }; }
        };

        /**
         * @brief 0.5*m*|v|^2 + 0.5*I*w^2. Shared by any oracle that reasons about energy.
         */
        float KineticEnergy(float a_Mass, const Math::Vector2& a_LinearVelocity,
                             float a_Inertia = 0.0f, float a_AngularVelocity = 0.0f);

        // ==================== Analytical ====================

        /**
         * @brief Ground truth for the engine's own integration scheme: semi-implicit
         * (symplectic) Euler, v += a*dt then x += v*dt each step — not the
         * continuous ½gt². Computes the exact discrete closed form
         * (x0 + n*v0*dt + a*dt^2*n*(n+1)/2) and compares against the engine's
         * actual output, so failures mean a genuine integration bug, not a
         * mismatch between continuous and discrete kinematics.
         */
        OracleResult AnalyticalFreeFallMatches(float a_ActualY, float a_Y0, float a_V0Y,
                                                float a_GravityY, float a_DeltaTime, int a_NumSteps,
                                                float a_Tolerance);

        /**
         * @brief For a head-on collision between two equal-mass bodies with
         * restitution 1, classical mechanics says velocities are exchanged.
         */
        OracleResult ElasticEqualMassCollisionSwapsVelocity(const Math::Vector2& a_VelocityABefore,
                                                              const Math::Vector2& a_VelocityBBefore,
                                                              const Math::Vector2& a_VelocityAAfter,
                                                              const Math::Vector2& a_VelocityBAfter,
                                                              float a_Tolerance);

        // ==================== Invariants ====================

        /**
         * @brief No contact should be resolved to penetrate deeper than the
         * solver's slop. Pass a manifold freshly recomputed from the final
         * transforms (not one cached mid-solve).
         */
        OracleResult NoPenetrationPastSlop(const Collision::Collision2DManifold& a_Manifold,
                                            float a_Slop = 0.01f, float a_Tolerance = 1e-3f);

        /**
         * @brief Sum(mass_i * velocity_i) must be conserved across a step with no
         * external force (gravity off, no joints) — contact impulses are
         * equal-and-opposite by construction, so this should hold even through
         * collisions.
         */
        OracleResult MomentumConserved(const std::vector<float>& a_Masses,
                                        const std::vector<Math::Vector2>& a_VelocitiesBefore,
                                        const std::vector<Math::Vector2>& a_VelocitiesAfter,
                                        float a_Tolerance);

        /**
         * @brief Total kinetic energy must not increase when every restitution
         * involved is < 1. A small tolerance absorbs the solver's bias/slop terms,
         * which can inject trace amounts of numerical energy.
         */
        OracleResult EnergyNonIncreasing(float a_EnergyBefore, float a_EnergyAfter, float a_Tolerance);

        /**
         * @brief A settled body's velocity should stay near zero for every
         * subsequent sample, not drift or explode (jitter).
         */
        OracleResult RestingBodyStaysAtRest(const std::vector<Math::Vector2>& a_LinearVelocitySamples,
                                             const std::vector<float>& a_AngularVelocitySamples,
                                             float a_VelocityEpsilon);

        /**
         * @brief |actual anchor distance - target| <= tolerance, or (for
         * maxDistanceOnly ropes) actual <= target + tolerance.
         */
        OracleResult DistanceJointSatisfied(const Math::Vector2& a_Anchor1, const Math::Vector2& a_Anchor2,
                                             float a_TargetDistance, bool a_MaxDistanceOnly, float a_Tolerance);

        // ==================== Metamorphic ====================

        /**
         * @brief Compares a run against a second run whose scene was mirrored
         * across the Y axis at setup (every initial x, vx, rotation and angular
         * velocity negated). Both snapshot vectors are assumed sorted by entity id
         * (RunScene's contract) and to correspond entity-for-entity. Gravity acts
         * along Y only, so Y motion is untouched by the mirror.
         */
        OracleResult MirroredScenesMatch(const std::vector<EntitySnapshot>& a_Original,
                                          const std::vector<EntitySnapshot>& a_MirroredX,
                                          float a_Tolerance);

        // ==================== Golden-master ====================

        /**
         * @brief Compares a run bit-for-bit against a recorded baseline on disk
         * (floats round-tripped via std::hexfloat, so no precision is lost in the
         * text form). If the file doesn't exist and a_RecordIfMissing is true, this
         * writes a_Actual as the new baseline and passes — the standard
         * golden-master bootstrap. Re-running after an intentional engine change
         * requires deleting the stale file so it can be re-recorded.
         */
        OracleResult MatchesGoldenMaster(const std::vector<EntitySnapshot>& a_Actual,
                                          const std::string& a_GoldenFilePath,
                                          bool a_RecordIfMissing = true);
    }
}