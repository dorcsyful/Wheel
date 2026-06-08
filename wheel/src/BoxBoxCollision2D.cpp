#include "../include/helpers/Collision2DManifold.h"
#include "systems/subsystems/BoxBoxCollision2D.h"

#include "math/Matrix2x2.h"
#include <algorithm>
#include <cmath>

Wheel::Engine::Collision::Collision2DManifold Wheel::Engine::Collision::BoxBoxCollision2D::BoxBoxCollision(
    const Components::BoxCollider2D& a_Collider1, const Components::Transform2D& a_Transform1, const Components::BoxCollider2D& a_Collider2, const Components::Transform2D& a_Transform2)
{
    const Math::Vector2* vertices1 = GetVertices(a_Collider1, a_Transform1);
    const Math::Vector2* vertices2 = GetVertices(a_Collider2, a_Transform2);

    Collision2DManifold manifold(a_Transform1.GetEntityId(), a_Transform2.GetEntityId());
    float minOverlap1 = FLT_MAX, minOverlap2 = FLT_MAX;
    int bestAxis1 = -1, bestAxis2 = -1;

    // Each box has 4 normals, but normals[2] == -normals[0] and normals[3] == -normals[1].
    // Projecting onto opposite axes gives the same overlap, so only 2 unique axes per box.
    for (int i = 0; i < 2; i++)
    {
        Math::Vector2 axis = a_Collider1.cachedNormals[i];

        float min1, max1, min2, max2;
        ProjectOntoAxis(vertices1, axis, min1, max1);
        ProjectOntoAxis(vertices2, axis, min2, max2);

        if (min1 >= max2 || min2 >= max1)
            return manifold;

        float overlap = std::min(max1, max2) - std::max(min1, min2);
        if (overlap < minOverlap1) { minOverlap1 = overlap; bestAxis1 = i; }
    }

    for (int i = 0; i < 2; i++)
    {
        Math::Vector2 axis = a_Collider2.cachedNormals[i];

        float min1, max1, min2, max2;
        ProjectOntoAxis(vertices1, axis, min1, max1);
        ProjectOntoAxis(vertices2, axis, min2, max2);

        if (min1 >= max2 || min2 >= max1)
            return manifold;

        float overlap = std::min(max1, max2) - std::max(min1, min2);
        if (overlap < minOverlap2) { minOverlap2 = overlap; bestAxis2 = i; }
    }

    if (minOverlap1 < minOverlap2)
    {
        manifold.collisionNormal = a_Collider1.cachedNormals[bestAxis1];
    }
    else
    {
        manifold.collisionNormal = a_Collider2.cachedNormals[bestAxis2];
    }
    Math::Vector2 direction = a_Transform2.GetPosition() - a_Transform1.GetPosition();
    if (direction.Dot(manifold.collisionNormal) < 0.0f)
        manifold.collisionNormal = manifold.collisionNormal * -1.0f;

    // Opposite normals project identically in SAT, so bestAxis may be the wrong face.
    // Correct it to whichever face actually faces the collision direction.
    if (a_Collider1.cachedNormals[bestAxis1].Dot(manifold.collisionNormal) < 0.0f)
        bestAxis1 = (bestAxis1 + 2) % 4;
    if (a_Collider2.cachedNormals[bestAxis2].Dot(manifold.collisionNormal) > 0.0f)
        bestAxis2 = (bestAxis2 + 2) % 4;

    manifold.contactPoint[0] = vertices1[bestAxis1];
    manifold.contactPoint[1] = vertices1[(bestAxis1 + 1) % 4];
    manifold.tangentNormal = manifold.collisionNormal.Perpendicular();
    manifold.isColliding = true;

    Math::Vector2 refVertex;
    // flip == true when collider2 supplies the reference face. It both keeps the
    // contact ID distinct across a reference/incident role swap and selects the
    // correct outward normal for the penetration measurement below.
    bool flip = !(minOverlap1 <= minOverlap2);
    if (minOverlap1 <= minOverlap2)
    {
        manifold.contactCount = GetContactPoints(a_Collider1, a_Collider2, bestAxis1, bestAxis2, manifold.contactPoint, manifold.contactId, flip);
        refVertex = a_Collider1.cachedVertices[bestAxis1];
    }
    else
    {
        manifold.contactCount = GetContactPoints(a_Collider2, a_Collider1, bestAxis2, bestAxis1, manifold.contactPoint, manifold.contactId, flip);
        refVertex = a_Collider2.cachedVertices[bestAxis2];
    }

    // Penetration is measured along the REFERENCE face's outward normal, which
    // points from the reference box toward the incident box. collisionNormal points
    // collider1 -> collider2, so that matches only when collider1 is the reference;
    // when collider2 is the reference (flip) the sign is reversed. Using
    // collisionNormal unconditionally made every collider2-reference contact report
    // zero penetration, so the position solver silently let it sink.
    Math::Vector2 refNormal = flip ? manifold.collisionNormal * -1.0f : manifold.collisionNormal;
    for (int i = 0; i < manifold.contactCount; i++)
        manifold.penetrationDepth[i] = std::max(0.0f, -refNormal.Dot(manifold.contactPoint[i] - refVertex));

    return manifold;
}

const Wheel::Math::Vector2* Wheel::Engine::Collision::BoxBoxCollision2D::GetVertices(
    const Components::BoxCollider2D& a_Collider, const Components::Transform2D& a_Transform2D)
{
    if (!a_Collider.isDirty)
        return a_Collider.cachedVertices;

    float halfWidth  = a_Collider.GetWidth()  / 2.0f;
    float halfHeight = a_Collider.GetHeight() / 2.0f;
    const Math::Vector2& offset   = a_Collider.GetOffset();
    const Math::Vector2& scale    = a_Transform2D.GetScale();
    const Math::Vector2& position = a_Transform2D.GetPosition();

    Math::Vector2 local[4] = {
        {-halfWidth * scale.x, -halfHeight * scale.y},
        { halfWidth * scale.x, -halfHeight * scale.y},
        { halfWidth * scale.x,  halfHeight * scale.y},
        {-halfWidth * scale.x,  halfHeight * scale.y}
    };

    Math::Matrix2x2 rotationMatrix;
    rotationMatrix.CreateRotation(a_Transform2D.GetRotationInRadians());
    for (int i = 0; i < 4; i++)
    {
        a_Collider.cachedVertices[i] = rotationMatrix * local[i] + offset + position;
    }

    a_Collider.cachedNormals[0] = rotationMatrix * Math::Vector2(0.0f, -1.0f);
    a_Collider.cachedNormals[1] = rotationMatrix * Math::Vector2( 1.0f, 0.0f);
    a_Collider.cachedNormals[2] = rotationMatrix * Math::Vector2( 0.0f,1.0f);
    a_Collider.cachedNormals[3] = rotationMatrix * Math::Vector2( -1.0f, 0.0f);

    a_Collider.isDirty = false;
    return a_Collider.cachedVertices;
}

int Wheel::Engine::Collision::BoxBoxCollision2D::GetContactPoints(const Components::BoxCollider2D& a_Collider1,
    const Components::BoxCollider2D& a_Collider2, int a_Normal1, int a_Normal2, Math::Vector2* a_ContactPoints, uint16_t* a_ContactIds, bool a_Flip)
{
    float lowestDot = FLT_MAX;
    int incidentIndex = 0;
    for (int i = 0; i < 4; i++) {
        float dot = a_Collider1.cachedNormals[a_Normal1].Dot(a_Collider2.cachedNormals[i]);
        if (dot < lowestDot) { lowestDot = dot; incidentIndex = i; }
    }

    // Building a 6 bit ID to be able to identify the collision in the next frame for warm starting
    auto packId = [&](int endpoint) -> uint16_t {
        return (uint16_t)(((a_Flip ? 1 : 0) << 5) | ((a_Normal1 & 3) << 3) | ((incidentIndex & 3) << 1) | (endpoint & 1));
    };

    Math::Vector2 referenceVertex1 = a_Collider1.cachedVertices[a_Normal1];
    Math::Vector2 referenceVertex2 = a_Collider1.cachedVertices[(a_Normal1 + 1) % 4];

    Math::Vector2 incidentVertex1 = a_Collider2.cachedVertices[incidentIndex];
    Math::Vector2 incidentVertex2 = a_Collider2.cachedVertices[(incidentIndex + 1) % 4];

    Math::Vector2 refTangent = a_Collider1.cachedNormals[a_Normal1].Perpendicular();
    float refLength = (referenceVertex1 - referenceVertex2).Length();

    float proj1 = (incidentVertex1 - referenceVertex1).Dot(refTangent);
    float proj2 = (incidentVertex2 - referenceVertex1).Dot(refTangent);
    float minProj = std::min(proj1, proj2);
    float maxProj = std::max(proj1, proj2);


    if (maxProj < 0)
    {
        a_ContactPoints[0] = referenceVertex1;
        a_ContactIds[0] = packId(0);
        return 1;
    }
    if (minProj > refLength)
    {
        a_ContactPoints[0] = referenceVertex2;
        a_ContactIds[0] = packId(1);
        return 1;
    }
    Math::Vector2 clippedPoints[2];
    ClipSegmentToLine(incidentVertex1, incidentVertex2, refTangent*(-1), referenceVertex1, clippedPoints);
    ClipSegmentToLine(clippedPoints[0], clippedPoints[1], refTangent, referenceVertex2, clippedPoints);
    int counter = 0;

    for (int k = 0; k < 2; k++) {
        const Math::Vector2& v = clippedPoints[k];
        if (a_Collider1.cachedNormals[a_Normal1].Dot(v - referenceVertex1) <= 0.002) {
            a_ContactPoints[counter] = v;
            a_ContactIds[counter] = packId(k);
            counter++;
        }
    }
    if (counter == 1)
    {
        a_ContactPoints[1] = a_ContactPoints[0];
        a_ContactIds[1] = a_ContactIds[0];
    }
    return counter;
}

void Wheel::Engine::Collision::BoxBoxCollision2D::ClipSegmentToLine(const Math::Vector2& a_P1,
    const Math::Vector2& a_P2, const Math::Vector2& a_Normal, Math::Vector2 a_Offset, Math::Vector2* a_Contact)
{
    float distance0 = (a_P1 - a_Offset).Dot(a_Normal);
    float distance1 = (a_P2 - a_Offset).Dot(a_Normal);
    // If the points are behind the plane, don't clip
    if (distance0 <= 0) a_Contact[0] = a_P1;
    if (distance1 <= 0) a_Contact[1] = a_P2;

    // If one is in front of the plane, have to clip it to the intersection point
    // clippedPoints.length < 2 for edge case where 1 point is exactly on the plane
    if (std::copysignf(1.0,distance0) != std::copysignf(1.0,distance1)) {
        float pctAcross = distance1 / (distance1 - distance0);
        Math::Vector2 intersectionPt = a_P2 + ((a_P1 - a_P2)*(pctAcross));
        if (distance0 > 0)
            a_Contact[0] = intersectionPt;
        else
            a_Contact[1] = intersectionPt;
    }

}

void Wheel::Engine::Collision::BoxBoxCollision2D::ProjectOntoAxis(const Math::Vector2* a_Vertices,
                                                                  const Math::Vector2& a_Axis, float& a_Min, float& a_Max)
{
    a_Min = FLT_MAX;
    a_Max = -FLT_MAX;
    for (int i = 0; i < 4; i++)
    {
        float dot = a_Axis.Dot(a_Vertices[i]);
        if (dot < a_Min) a_Min = dot;
        if (dot > a_Max) a_Max = dot;
    }
}
