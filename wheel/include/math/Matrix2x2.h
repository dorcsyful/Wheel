#pragma once
#include "Vector2.h"

namespace Wheel
{
    namespace Math
    {
        class Matrix2x2
        {
        public:
            Matrix2x2() { m_Values[0][0] = 1.0f; m_Values[0][1] = 0.0f; m_Values[1][0] = 0.0f; m_Values[1][1] = 1.0f; }
            Matrix2x2(float a_00, float a_01, float a_10, float a_11)
            {
                m_Values[0][0] = a_00; m_Values[0][1] = a_01;
                m_Values[1][0] = a_10; m_Values[1][1] = a_11;
            }
            float& First() { return m_Values[0][0]; }
            Matrix2x2 operator*(const Matrix2x2& a_Other)
            {
                Matrix2x2 result;
                for (int i = 0; i < 2; ++i)
                    for (int j = 0; j < 2; ++j)                    {
                        float sum = 0.0f;
                        for (int k = 0; k < 2; ++k)
                            sum += m_Values[i][k] * a_Other.m_Values[k][j];
                        result.m_Values[i][j] = sum;
                    }
                return result;
            }
            Vector2 operator*(const Vector2& a_Other)
            {
                Vector2 result;
                result.x = m_Values[0][0] * a_Other.x + m_Values[0][1] * a_Other.y;
                result.y = m_Values[1][0] * a_Other.x + m_Values[1][1] * a_Other.y;
                return result;
            }

            Math::Vector2& operator[](int index) { return m_Values[index]; }

            void CreateRotation(float a_Angle)
            {
                float s = sin(a_Angle);
                float c = cos(a_Angle);
                m_Values[0][0] = c; m_Values[0][1] = -s;
                m_Values[1][0] = s; m_Values[1][1] = c;
            }

            Matrix2x2 Inverse() const
            {
                float det = m_Values[0][0] * m_Values[1][1] - m_Values[0][1] * m_Values[1][0];
                if (det == 0.0f)
                    return Matrix2x2(); // Return identity if not invertible
                float invDet = 1.0f / det;
                return Matrix2x2(
                    m_Values[1][1] * invDet, -m_Values[0][1] * invDet,
                    -m_Values[1][0] * invDet, m_Values[0][0] * invDet
                );
            }

        private:
            Vector2 m_Values[2];
        };
    }
}
