#pragma once

namespace Wheel
{
    namespace Math
    {
        /**
         * @brief Row-major 4x4 matrix
         */
        class Matrix4x4
        {
        public:
            Matrix4x4() = default;
            Matrix4x4(float a_00, float a_01, float a_02, float a_03,
                      float a_10, float a_11, float a_12, float a_13,
                      float a_20, float a_21, float a_22, float a_23,
                      float a_30, float a_31, float a_32, float a_33)
            {
                m_Values[0][0] = a_00; m_Values[0][1] = a_01; m_Values[0][2] = a_02; m_Values[0][3] = a_03;
                m_Values[1][0] = a_10; m_Values[1][1] = a_11; m_Values[1][2] = a_12; m_Values[1][3] = a_13;
                m_Values[2][0] = a_20; m_Values[2][1] = a_21; m_Values[2][2] = a_22; m_Values[2][3] = a_23;
                m_Values[3][0] = a_30; m_Values[3][1] = a_31; m_Values[3][2] = a_32; m_Values[3][3] = a_33;
            }
            float& First() { return m_Values[0][0]; }

            Matrix4x4 operator*(const Matrix4x4& a_Other) const
            {
                Matrix4x4 result;
                for (int i = 0; i < 4; ++i)
                    for (int j = 0; j < 4; ++j)
                    {
                        float sum = 0.0f;
                        for (int k = 0; k < 4; ++k)
                            sum += m_Values[i][k] * a_Other.m_Values[k][j];
                        result.m_Values[i][j] = sum;
                    }
                return result;
            }
            [[nodiscard]] Matrix4x4  Transpose() const
            {
                return Matrix4x4(
                    m_Values[0][0], m_Values[1][0], m_Values[2][0], m_Values[3][0],
                    m_Values[0][1], m_Values[1][1], m_Values[2][1], m_Values[3][1],
                    m_Values[0][2], m_Values[1][2], m_Values[2][2], m_Values[3][2],
                    m_Values[0][3], m_Values[1][3], m_Values[2][3], m_Values[3][3]
                );
            }


        private:
            float m_Values[4][4]{};
        };
    }
}