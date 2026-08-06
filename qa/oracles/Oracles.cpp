#include "Oracles.h"

#include <cmath>
#include <cstring>
#include <fstream>
#include <sstream>

using Wheel::Math::Vector2;

namespace
{
    uint32_t FloatToBits(float a_Value)
    {
        static_assert(sizeof(float) == sizeof(uint32_t), "expects IEEE-754 single precision float");
        uint32_t bits;
        std::memcpy(&bits, &a_Value, sizeof(bits));
        return bits;
    }

    float FloatFromBits(uint32_t a_Bits)
    {
        float value;
        std::memcpy(&value, &a_Bits, sizeof(value));
        return value;
    }
    bool NearFloat(float a_A, float a_B, float a_Tolerance)
    {
        return std::abs(a_A - a_B) <= a_Tolerance;
    }

    bool NearVector2(const Vector2& a_A, const Vector2& a_B, float a_Tolerance)
    {
        return NearFloat(a_A.x, a_B.x, a_Tolerance) && NearFloat(a_A.y, a_B.y, a_Tolerance);
    }

    std::string FormatVector2(const Vector2& a_V)
    {
        std::ostringstream oss;
        oss << "(" << a_V.x << ", " << a_V.y << ")";
        return oss.str();
    }
}

float Wheel::QA::KineticEnergy(float a_Mass, const Math::Vector2& a_LinearVelocity, float a_Inertia, float a_AngularVelocity)
{
    return 0.5f * a_Mass * a_LinearVelocity.LengthSquared() + 0.5f * a_Inertia * a_AngularVelocity * a_AngularVelocity;
}

Wheel::QA::OracleResult Wheel::QA::AnalyticalFreeFallMatches(float a_ActualY, float a_Y0, float a_V0Y,
    float a_GravityY, float a_DeltaTime, int a_NumSteps, float a_Tolerance)
{
    double n = static_cast<double>(a_NumSteps);
    double expectedY = a_Y0 + n * a_V0Y * a_DeltaTime
                      + a_GravityY * a_DeltaTime * a_DeltaTime * n * (n + 1.0) / 2.0;

    if (!NearFloat(a_ActualY, static_cast<float>(expectedY), a_Tolerance))
    {
        std::ostringstream oss;
        oss << "AnalyticalFreeFallMatches: expected y=" << expectedY << " (discrete semi-implicit-Euler closed form), "
            << "actual y=" << a_ActualY << ", |diff|=" << std::abs(a_ActualY - expectedY)
            << " exceeds tolerance " << a_Tolerance;
        return OracleResult::Fail(oss.str());
    }
    return OracleResult::Pass();
}

Wheel::QA::OracleResult Wheel::QA::ElasticEqualMassCollisionSwapsVelocity(const Math::Vector2& a_VelocityABefore,
    const Math::Vector2& a_VelocityBBefore, const Math::Vector2& a_VelocityAAfter, const Math::Vector2& a_VelocityBAfter,
    float a_Tolerance)
{
    if (!NearVector2(a_VelocityAAfter, a_VelocityBBefore, a_Tolerance) ||
        !NearVector2(a_VelocityBAfter, a_VelocityABefore, a_Tolerance))
    {
        std::ostringstream oss;
        oss << "ElasticEqualMassCollisionSwapsVelocity: expected A after " << FormatVector2(a_VelocityBBefore)
            << " (B's velocity before), got " << FormatVector2(a_VelocityAAfter)
            << "; expected B after " << FormatVector2(a_VelocityABefore)
            << " (A's velocity before), got " << FormatVector2(a_VelocityBAfter);
        return OracleResult::Fail(oss.str());
    }
    return OracleResult::Pass();
}

Wheel::QA::OracleResult Wheel::QA::NoPenetrationPastSlop(const Collision::Collision2DManifold& a_Manifold,
    float a_Slop, float a_Tolerance)
{
    if (!a_Manifold.isColliding) return OracleResult::Pass();

    for (int i = 0; i < a_Manifold.contactCount; i++)
    {
        if (a_Manifold.penetrationDepth[i] > a_Slop + a_Tolerance)
        {
            std::ostringstream oss;
            oss << "NoPenetrationPastSlop: contact " << i << " penetration=" << a_Manifold.penetrationDepth[i]
                << " exceeds slop(" << a_Slop << ") + tolerance(" << a_Tolerance << ")";
            return OracleResult::Fail(oss.str());
        }
    }
    return OracleResult::Pass();
}

Wheel::QA::OracleResult Wheel::QA::MomentumConserved(const std::vector<float>& a_Masses,
    const std::vector<Math::Vector2>& a_VelocitiesBefore, const std::vector<Math::Vector2>& a_VelocitiesAfter,
    float a_Tolerance)
{
    if (a_Masses.size() != a_VelocitiesBefore.size() || a_Masses.size() != a_VelocitiesAfter.size())
        return OracleResult::Fail("MomentumConserved: masses/velocitiesBefore/velocitiesAfter size mismatch");

    Vector2 momentumBefore, momentumAfter;
    for (size_t i = 0; i < a_Masses.size(); i++)
    {
        momentumBefore += a_VelocitiesBefore[i] * a_Masses[i];
        momentumAfter  += a_VelocitiesAfter[i]  * a_Masses[i];
    }

    if (!NearVector2(momentumBefore, momentumAfter, a_Tolerance))
    {
        std::ostringstream oss;
        oss << "MomentumConserved: before=" << FormatVector2(momentumBefore)
            << " after=" << FormatVector2(momentumAfter)
            << " |diff|=" << (momentumAfter - momentumBefore).Length() << " exceeds tolerance " << a_Tolerance;
        return OracleResult::Fail(oss.str());
    }
    return OracleResult::Pass();
}

Wheel::QA::OracleResult Wheel::QA::EnergyNonIncreasing(float a_EnergyBefore, float a_EnergyAfter, float a_Tolerance)
{
    if (a_EnergyAfter > a_EnergyBefore + a_Tolerance)
    {
        std::ostringstream oss;
        oss << "EnergyNonIncreasing: before=" << a_EnergyBefore << " after=" << a_EnergyAfter
            << " increased by " << (a_EnergyAfter - a_EnergyBefore) << ", exceeds tolerance " << a_Tolerance;
        return OracleResult::Fail(oss.str());
    }
    return OracleResult::Pass();
}

Wheel::QA::OracleResult Wheel::QA::RestingBodyStaysAtRest(const std::vector<Math::Vector2>& a_LinearVelocitySamples,
    const std::vector<float>& a_AngularVelocitySamples, float a_VelocityEpsilon)
{
    for (size_t i = 0; i < a_LinearVelocitySamples.size(); i++)
    {
        if (a_LinearVelocitySamples[i].Length() > a_VelocityEpsilon)
        {
            std::ostringstream oss;
            oss << "RestingBodyStaysAtRest: sample " << i << " linear velocity "
                << FormatVector2(a_LinearVelocitySamples[i]) << " exceeds epsilon " << a_VelocityEpsilon;
            return OracleResult::Fail(oss.str());
        }
    }
    for (size_t i = 0; i < a_AngularVelocitySamples.size(); i++)
    {
        if (std::abs(a_AngularVelocitySamples[i]) > a_VelocityEpsilon)
        {
            std::ostringstream oss;
            oss << "RestingBodyStaysAtRest: sample " << i << " angular velocity "
                << a_AngularVelocitySamples[i] << " exceeds epsilon " << a_VelocityEpsilon;
            return OracleResult::Fail(oss.str());
        }
    }
    return OracleResult::Pass();
}

Wheel::QA::OracleResult Wheel::QA::DistanceJointSatisfied(const Math::Vector2& a_Anchor1, const Math::Vector2& a_Anchor2,
    float a_TargetDistance, bool a_MaxDistanceOnly, float a_Tolerance)
{
    float actual = (a_Anchor1 - a_Anchor2).Length();

    bool ok = a_MaxDistanceOnly ? (actual <= a_TargetDistance + a_Tolerance)
                                 : NearFloat(actual, a_TargetDistance, a_Tolerance);
    if (!ok)
    {
        std::ostringstream oss;
        oss << "DistanceJointSatisfied: actual distance=" << actual << " target=" << a_TargetDistance
            << (a_MaxDistanceOnly ? " (max-distance-only, actual must be <= target + tolerance)" : "")
            << " tolerance=" << a_Tolerance;
        return OracleResult::Fail(oss.str());
    }
    return OracleResult::Pass();
}

Wheel::QA::OracleResult Wheel::QA::MirroredScenesMatch(const std::vector<EntitySnapshot>& a_Original,
    const std::vector<EntitySnapshot>& a_MirroredX, float a_Tolerance)
{
    if (a_Original.size() != a_MirroredX.size())
        return OracleResult::Fail("MirroredScenesMatch: snapshot count mismatch");

    for (size_t i = 0; i < a_Original.size(); i++)
    {
        const EntitySnapshot& o = a_Original[i];
        const EntitySnapshot& m = a_MirroredX[i];
        if (o.entityId != m.entityId)
            return OracleResult::Fail("MirroredScenesMatch: entity id mismatch at index " + std::to_string(i));

        bool ok = NearFloat(m.position.x, -o.position.x, a_Tolerance)
               && NearFloat(m.position.y,  o.position.y, a_Tolerance)
               && NearFloat(m.rotation,   -o.rotation,   a_Tolerance)
               && NearFloat(m.linearVelocity.x, -o.linearVelocity.x, a_Tolerance)
               && NearFloat(m.linearVelocity.y,  o.linearVelocity.y, a_Tolerance)
               && NearFloat(m.angularVelocity,  -o.angularVelocity,  a_Tolerance);

        if (!ok)
        {
            std::ostringstream oss;
            oss << "MirroredScenesMatch: entity " << o.entityId
                << " original pos=" << FormatVector2(o.position) << " rot=" << o.rotation
                << " vs mirrored pos=" << FormatVector2(m.position) << " rot=" << m.rotation
                << " (expected mirrored pos.x=" << -o.position.x << ", rot=" << -o.rotation << ")";
            return OracleResult::Fail(oss.str());
        }
    }
    return OracleResult::Pass();
}

Wheel::QA::OracleResult Wheel::QA::MatchesGoldenMaster(const std::vector<EntitySnapshot>& a_Actual,
    const std::string& a_GoldenFilePath, bool a_RecordIfMissing)
{
    std::ifstream in(a_GoldenFilePath);
    if (!in.good())
    {
        if (!a_RecordIfMissing)
            return OracleResult::Fail("MatchesGoldenMaster: no golden file at " + a_GoldenFilePath);

        std::ofstream out(a_GoldenFilePath);
        if (!out.good())
            return OracleResult::Fail("MatchesGoldenMaster: could not create golden file at " + a_GoldenFilePath);

        out << std::hex;
        for (const auto& s : a_Actual)
        {
            out << s.entityId << ' ' << FloatToBits(s.position.x) << ' ' << FloatToBits(s.position.y) << ' '
                << FloatToBits(s.rotation) << ' ' << FloatToBits(s.linearVelocity.x) << ' '
                << FloatToBits(s.linearVelocity.y) << ' ' << FloatToBits(s.angularVelocity) << '\n';
        }
        return OracleResult::Fail("MatchesGoldenMaster: no baseline existed, recorded a new one at " +
            a_GoldenFilePath + " (re-run to actually compare)");
    }

    in >> std::hex;
    std::vector<EntitySnapshot> recorded;
    EntitySnapshot s;
    while (in >> s.entityId)
    {
        uint32_t bits[6];
        bool ok = true;
        for (int i = 0; i < 6 && ok; i++) ok = static_cast<bool>(in >> bits[i]);
        if (!ok) break;

        s.position.x = FloatFromBits(bits[0]);
        s.position.y = FloatFromBits(bits[1]);
        s.rotation = FloatFromBits(bits[2]);
        s.linearVelocity.x = FloatFromBits(bits[3]);
        s.linearVelocity.y = FloatFromBits(bits[4]);
        s.angularVelocity = FloatFromBits(bits[5]);
        recorded.push_back(s);
    }

    if (recorded.size() != a_Actual.size())
    {
        std::ostringstream oss;
        oss << "MatchesGoldenMaster: recorded " << recorded.size() << " entities, actual has " << a_Actual.size();
        return OracleResult::Fail(oss.str());
    }

    for (size_t i = 0; i < recorded.size(); i++)
    {
        const EntitySnapshot& r = recorded[i];
        const EntitySnapshot& a = a_Actual[i];
        bool identical = r.entityId == a.entityId
            && r.position.x == a.position.x && r.position.y == a.position.y
            && r.rotation == a.rotation
            && r.linearVelocity.x == a.linearVelocity.x && r.linearVelocity.y == a.linearVelocity.y
            && r.angularVelocity == a.angularVelocity;

        if (!identical)
        {
            std::ostringstream oss;
            oss << "MatchesGoldenMaster: entity " << i << " diverged from recorded baseline in " << a_GoldenFilePath
                << " (bit-for-bit mismatch)";
            return OracleResult::Fail(oss.str());
        }
    }
    return OracleResult::Pass();
}