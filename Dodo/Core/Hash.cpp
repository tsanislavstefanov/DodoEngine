#include "pch.h"
#include "Hash.h"

namespace Dodo {

    template<typename>
    struct TraitsFNV
    {};

    template<>
    struct TraitsFNV<uint32_t>
    {
        static constexpr uint32_t Prime = 16777619;
        static constexpr uint32_t OffsetBasis = 2166136261;
    };

    template<>
    struct TraitsFNV<uint64_t>
    {
        static constexpr uint64_t Prime = 1099511628211;
        static constexpr uint64_t OffsetBasis = 14695981039346656037;
    };

    size_t FNV1a::operator()(const std::string& octets)
    {
        // Reference for the algorithm and the values for
        // prime and offset basis online: http://www.isthe.com/chongo/tech/comp/fnv/.
        using FNV = TraitsFNV<size_t>;
        size_t hash = FNV::OffsetBasis;
        for (char octet : octets)
        {
            hash ^= octet;
            hash *= FNV::Prime;
        }

        return hash;
    }

}