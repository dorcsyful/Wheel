#pragma once

namespace Wheel
{
    namespace Math
    {
        /**
         * @brief Column-major 4x4 matrix
         */
        class Matrix4x4
        {
        public:
            Matrix4x4() = default;
            Matrix4x4(float a_00, float a_10, float a_20, float a_30,
                      float a_01, float a_11, float a_21, float a_31,
                      float a_02, float a_12, float a_22, float a_32,
                      float a_03, float a_13, float a_23, float a_33)
            {
                m_Values[0] = a_00; m_Values[4] = a_01; m_Values[8] = a_02; m_Values[12] = a_03;
                m_Values[1] = a_10; m_Values[5] = a_11; m_Values[9] = a_12; m_Values[13] = a_13;
                m_Values[2] = a_20; m_Values[6] = a_21; m_Values[10] = a_22; m_Values[14] = a_23;
                m_Values[3] = a_30; m_Values[7] = a_31; m_Values[11] = a_32; m_Values[15] = a_33;
            }
            float m_Values[16];
        };
    }
}