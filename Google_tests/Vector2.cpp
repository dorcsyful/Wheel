#include <gtest/gtest.h>
#include <cmath>
#include "../wheel/include/math/Vector2.h"

using Wheel::Math::Vector2;

// ==================== Constructors ====================

TEST(Vector2, DefaultConstructor_ZeroXY)
{
    Vector2 v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
}

TEST(Vector2, ParameterizedConstructor_SetsXY)
{
    Vector2 v(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 4.0f);
}

TEST(Vector2, CopyConstructor_CopiesXY)
{
    Vector2 a(1.5f, 2.5f);
    Vector2 b(a);
    EXPECT_FLOAT_EQ(b.x, 1.5f);
    EXPECT_FLOAT_EQ(b.y, 2.5f);
}

TEST(Vector2, CopyConstructor_IsIndependent)
{
    Vector2 a(1.0f, 2.0f);
    Vector2 b(a);
    b.x = 99.0f;
    EXPECT_FLOAT_EQ(a.x, 1.0f);
}

// ==================== Index Operator ====================

TEST(Vector2, IndexOperator_ReturnsX)
{
    Vector2 v(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v[0], 3.0f);
}

TEST(Vector2, IndexOperator_ReturnsY)
{
    Vector2 v(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v[1], 4.0f);
}

// ==================== Unary Negation ====================

TEST(Vector2, UnaryNegation_NegatesXY)
{
    Vector2 v(3.0f, -4.0f);
    Vector2 neg = -v;
    EXPECT_FLOAT_EQ(neg.x, -3.0f);
    EXPECT_FLOAT_EQ(neg.y,  4.0f);
}

TEST(Vector2, UnaryNegation_ZeroVector_RemainsZero)
{
    Vector2 neg = -Vector2();
    EXPECT_FLOAT_EQ(neg.x, 0.0f);
    EXPECT_FLOAT_EQ(neg.y, 0.0f);
}

// ==================== Addition / Subtraction ====================

TEST(Vector2, Addition_SumsComponents)
{
    Vector2 c = Vector2(1.0f, 2.0f) + Vector2(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(c.x, 4.0f);
    EXPECT_FLOAT_EQ(c.y, 6.0f);
}

TEST(Vector2, Addition_WithZero_ReturnsOriginal)
{
    Vector2 v(5.0f, 7.0f);
    Vector2 r = v + Vector2();
    EXPECT_FLOAT_EQ(r.x, 5.0f);
    EXPECT_FLOAT_EQ(r.y, 7.0f);
}

TEST(Vector2, Subtraction_DiffsComponents)
{
    Vector2 c = Vector2(5.0f, 3.0f) - Vector2(2.0f, 1.0f);
    EXPECT_FLOAT_EQ(c.x, 3.0f);
    EXPECT_FLOAT_EQ(c.y, 2.0f);
}

TEST(Vector2, Subtraction_SelfYieldsZero)
{
    Vector2 v(3.0f, 4.0f);
    Vector2 r = v - v;
    EXPECT_FLOAT_EQ(r.x, 0.0f);
    EXPECT_FLOAT_EQ(r.y, 0.0f);
}

// ==================== Scalar Multiplication / Division ====================

TEST(Vector2, MultiplyByScalar_ScalesComponents)
{
    Vector2 r = Vector2(2.0f, 3.0f) * 4.0f;
    EXPECT_FLOAT_EQ(r.x, 8.0f);
    EXPECT_FLOAT_EQ(r.y, 12.0f);
}

TEST(Vector2, MultiplyByZero_YieldsZeroVector)
{
    Vector2 r = Vector2(5.0f, 7.0f) * 0.0f;
    EXPECT_FLOAT_EQ(r.x, 0.0f);
    EXPECT_FLOAT_EQ(r.y, 0.0f);
}

TEST(Vector2, MultiplyByNegativeScalar_NegatesComponents)
{
    Vector2 r = Vector2(2.0f, 3.0f) * -1.0f;
    EXPECT_FLOAT_EQ(r.x, -2.0f);
    EXPECT_FLOAT_EQ(r.y, -3.0f);
}

TEST(Vector2, DivideByScalar_ScalesComponents)
{
    Vector2 r = Vector2(6.0f, 4.0f) / 2.0f;
    EXPECT_FLOAT_EQ(r.x, 3.0f);
    EXPECT_FLOAT_EQ(r.y, 2.0f);
}

TEST(Vector2, DivideByOne_ReturnsOriginal)
{
    Vector2 v(3.0f, 5.0f);
    Vector2 r = v / 1.0f;
    EXPECT_FLOAT_EQ(r.x, 3.0f);
    EXPECT_FLOAT_EQ(r.y, 5.0f);
}

// ==================== Compound Assignment Operators ====================

TEST(Vector2, PlusEquals_AddsInPlace)
{
    Vector2 v(1.0f, 2.0f);
    v += Vector2(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v.x, 4.0f);
    EXPECT_FLOAT_EQ(v.y, 6.0f);
}

TEST(Vector2, MinusEquals_SubtractsInPlace)
{
    Vector2 v(5.0f, 6.0f);
    v -= Vector2(1.0f, 2.0f);
    EXPECT_FLOAT_EQ(v.x, 4.0f);
    EXPECT_FLOAT_EQ(v.y, 4.0f);
}

TEST(Vector2, TimesEquals_ScalesInPlace)
{
    Vector2 v(2.0f, 3.0f);
    v *= 3.0f;
    EXPECT_FLOAT_EQ(v.x, 6.0f);
    EXPECT_FLOAT_EQ(v.y, 9.0f);
}

TEST(Vector2, DivideEquals_ScalesInPlace)
{
    Vector2 v(8.0f, 4.0f);
    v /= 2.0f;
    EXPECT_FLOAT_EQ(v.x, 4.0f);
    EXPECT_FLOAT_EQ(v.y, 2.0f);
}

TEST(Vector2, AssignmentOperator_CopiesXY)
{
    Vector2 b;
    b = Vector2(5.0f, 7.0f);
    EXPECT_FLOAT_EQ(b.x, 5.0f);
    EXPECT_FLOAT_EQ(b.y, 7.0f);
}

TEST(Vector2, AssignmentOperator_ChainAssignment)
{
    Vector2 a(1.0f, 2.0f), b, c;
    c = b = a;
    EXPECT_FLOAT_EQ(c.x, 1.0f);
    EXPECT_FLOAT_EQ(c.y, 2.0f);
}

// ==================== Equality ====================

TEST(Vector2, Equality_SameComponents_ReturnsTrue)
{
    EXPECT_TRUE(Vector2(1.0f, 2.0f) == Vector2(1.0f, 2.0f));
}

TEST(Vector2, Equality_DiffX_ReturnsFalse)
{
    EXPECT_FALSE(Vector2(1.0f, 2.0f) == Vector2(9.0f, 2.0f));
}

TEST(Vector2, Equality_DiffY_ReturnsFalse)
{
    EXPECT_FALSE(Vector2(1.0f, 2.0f) == Vector2(1.0f, 9.0f));
}

TEST(Vector2, Inequality_DiffComponents_ReturnsTrue)
{
    EXPECT_TRUE(Vector2(1.0f, 2.0f) != Vector2(3.0f, 4.0f));
}

TEST(Vector2, Inequality_SameComponents_ReturnsFalse)
{
    EXPECT_FALSE(Vector2(1.0f, 2.0f) != Vector2(1.0f, 2.0f));
}

// ==================== Dot Product ====================

TEST(Vector2, Dot_OrthogonalVectors_ReturnsZero)
{
    EXPECT_FLOAT_EQ(Vector2(1.0f, 0.0f).Dot(Vector2(0.0f, 1.0f)), 0.0f);
}

TEST(Vector2, Dot_SameVector_ReturnsLengthSquared)
{
    Vector2 v(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v.Dot(v), v.LengthSquared());
}

TEST(Vector2, Dot_GeneralCase)
{
    // 2*4 + 3*5 = 23
    EXPECT_FLOAT_EQ(Vector2(2.0f, 3.0f).Dot(Vector2(4.0f, 5.0f)), 23.0f);
}

TEST(Vector2, Dot_IsCommutative)
{
    Vector2 a(1.0f, 2.0f), b(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(a.Dot(b), b.Dot(a));
}

// ==================== Cross Product ====================

TEST(Vector2, Cross_ParallelVectors_ReturnsZero)
{
    EXPECT_FLOAT_EQ(Vector2(2.0f, 0.0f).Cross(Vector2(4.0f, 0.0f)), 0.0f);
}

TEST(Vector2, Cross_UnitXCrossUnitY_ReturnsOne)
{
    EXPECT_FLOAT_EQ(Vector2(1.0f, 0.0f).Cross(Vector2(0.0f, 1.0f)), 1.0f);
}

TEST(Vector2, Cross_ReversedOrder_NegatesResult)
{
    float ab = Vector2(1.0f, 0.0f).Cross(Vector2(0.0f, 1.0f));
    float ba = Vector2(0.0f, 1.0f).Cross(Vector2(1.0f, 0.0f));
    EXPECT_FLOAT_EQ(ab, -ba);
}

TEST(Vector2, Cross_GeneralCase)
{
    // 2*5 - 3*4 = -2
    EXPECT_FLOAT_EQ(Vector2(2.0f, 3.0f).Cross(Vector2(4.0f, 5.0f)), -2.0f);
}

// ==================== Length ====================

TEST(Vector2, Length_ZeroVector_ReturnsZero)
{
    EXPECT_FLOAT_EQ(Vector2().Length(), 0.0f);
}

TEST(Vector2, Length_UnitX_ReturnsOne)
{
    EXPECT_FLOAT_EQ(Vector2(1.0f, 0.0f).Length(), 1.0f);
}

TEST(Vector2, Length_3_4_Returns5)
{
    EXPECT_FLOAT_EQ(Vector2(3.0f, 4.0f).Length(), 5.0f);
}

TEST(Vector2, Length_NegativeComponents_CorrectResult)
{
    EXPECT_FLOAT_EQ(Vector2(-3.0f, -4.0f).Length(), 5.0f);
}

// ==================== LengthSquared ====================

TEST(Vector2, LengthSquared_ZeroVector_ReturnsZero)
{
    EXPECT_FLOAT_EQ(Vector2().LengthSquared(), 0.0f);
}

TEST(Vector2, LengthSquared_3_4_Returns25)
{
    EXPECT_FLOAT_EQ(Vector2(3.0f, 4.0f).LengthSquared(), 25.0f);
}

TEST(Vector2, LengthSquared_EqualsLengthTimesLength)
{
    Vector2 v(2.0f, 5.0f);
    float len = v.Length();
    EXPECT_FLOAT_EQ(v.LengthSquared(), len * len);
}

// ==================== Normalized ====================

TEST(Vector2, Normalized_ResultHasUnitLength)
{
    Vector2 n = Vector2(3.0f, 4.0f).Normalized();
    EXPECT_NEAR(n.Length(), 1.0f, 1e-6f);
}

TEST(Vector2, Normalized_DirectionPreserved)
{
    Vector2 n = Vector2(3.0f, 4.0f).Normalized();
    EXPECT_NEAR(n.x, 0.6f, 1e-6f);
    EXPECT_NEAR(n.y, 0.8f, 1e-6f);
}

TEST(Vector2, Normalized_AlreadyUnit_RemainsUnit)
{
    Vector2 n = Vector2(1.0f, 0.0f).Normalized();
    EXPECT_NEAR(n.x, 1.0f, 1e-6f);
    EXPECT_NEAR(n.y, 0.0f, 1e-6f);
}

TEST(Vector2, Normalized_DoesNotModifyOriginal)
{
    Vector2 v(3.0f, 4.0f);
    v.Normalized();
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 4.0f);
}

// ==================== Static Normalize ====================

TEST(Vector2, StaticNormalize_ResultHasUnitLength)
{
    Vector2 n = Vector2::Normalize(Vector2(0.0f, 5.0f));
    EXPECT_NEAR(n.Length(), 1.0f, 1e-6f);
}

TEST(Vector2, StaticNormalize_MatchesInstanceMethod)
{
    Vector2 v(3.0f, 4.0f);
    Vector2 n1 = v.Normalized();
    Vector2 n2 = Vector2::Normalize(v);
    EXPECT_NEAR(n1.x, n2.x, 1e-6f);
    EXPECT_NEAR(n1.y, n2.y, 1e-6f);
}

// ==================== Perpendicular ====================

TEST(Vector2, Perpendicular_DotWithOriginalIsZero)
{
    Vector2 v(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v.Dot(v.Perpendicular()), 0.0f);
}

TEST(Vector2, Perpendicular_SameLengthAsOriginal)
{
    Vector2 v(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v.Perpendicular().Length(), v.Length());
}

TEST(Vector2, Perpendicular_UnitX_YieldsUnitY)
{
    Vector2 p = Vector2(1.0f, 0.0f).Perpendicular();
    EXPECT_FLOAT_EQ(p.x, 0.0f);
    EXPECT_FLOAT_EQ(p.y, 1.0f);
}

TEST(Vector2, Perpendicular_AppliedTwice_NegatesOriginal)
{
    Vector2 v(3.0f, 4.0f);
    Vector2 pp = v.Perpendicular().Perpendicular();
    EXPECT_FLOAT_EQ(pp.x, -v.x);
    EXPECT_FLOAT_EQ(pp.y, -v.y);
}