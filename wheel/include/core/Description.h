#pragma once
#include <cassert>
#include <cstdint>
#include <bitset>
#include "Globals.h"

inline size_t get_first_set_bit(const std::bitset<MAX_COMPONENT_TYPES>& b)
{
    if (b.none()) return MAX_COMPONENT_TYPES; // Or handle as needed

    unsigned long long val = b.to_ullong();

#ifdef _MSC_VER
    unsigned long index;
    _BitScanForward64(&index, val);
    return static_cast<size_t>(index);
#else
    return static_cast<size_t>(__builtin_ctzll(val));
#endif
}


namespace Wheel
{
    namespace Engine
    {
        class Description
        {
        public:
            Description() = default;
            Description(const Description& a_Other) { m_ComponentBitset = a_Other.m_ComponentBitset; };
            Description(Description&& a_Other) noexcept { m_ComponentBitset = a_Other.m_ComponentBitset; };

            const std::bitset<MAX_COMPONENT_TYPES>& GetAsBitset()
            {
                return m_ComponentBitset;
            }
            void AddComponentType(std::bitset<MAX_COMPONENT_TYPES> componentBitset)
            {
                size_t firstBit = get_first_set_bit(componentBitset);
                assert(firstBit < MAX_COMPONENT_TYPES);

                assert(!m_ComponentBitset[firstBit]);
                m_ComponentBitset[firstBit] = true;
            }
            void RemoveComponentType(std::bitset<MAX_COMPONENT_TYPES> componentBitset)
            {
                size_t firstBit = get_first_set_bit(componentBitset);
                assert(m_ComponentBitset[firstBit] );
                m_ComponentBitset[firstBit] = false;

            }
            bool HasComponentType(Description& componentBitset) const
            {
                size_t firstBit = get_first_set_bit(componentBitset.GetAsBitset());
                assert(firstBit < MAX_COMPONENT_TYPES && "Component type out of range");
                return m_ComponentBitset[firstBit] == true;

            }
            bool IsEmpty()
            {
                for (int i = 0; i < MAX_COMPONENT_TYPES; i++)
                {
                    if (m_ComponentBitset[i] == true)
                    {
                        return false;
                    }
                }
                return true;
            }
            void operator=(const Description& other)
            {
                m_ComponentBitset = other.m_ComponentBitset;
            }
            void Reset()
            {
                m_ComponentBitset = std::bitset<MAX_COMPONENT_TYPES>();
            }

        private:
            std::bitset<MAX_COMPONENT_TYPES> m_ComponentBitset;
        };
    }
}