#include "pch.h"
#include "Hash.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // FNV TRAITS //////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    template<typename>
    struct FnvTraits
    {};

    template<>
    struct FnvTraits<uint32_t>
    {
        static constexpr uint32_t Prime = 16777619;
        static constexpr uint32_t OffsetBasis = 2166136261;
    };

    template<>
    struct FnvTraits<uint64_t>
    {
        static constexpr uint64_t Prime = 1099511628211;
        static constexpr uint64_t OffsetBasis = 14695981039346656037;
    };

    ////////////////////////////////////////////////////////////////
    // HASH ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    size_t Hash::GenerateFnv1a(const std::string& octets)
    {
        // Reference for the algorithm and the values for
        // prime and offset basis found online: http://www.isthe.com/chongo/tech/comp/fnv/.
        using Fnv = FnvTraits<size_t>;
        size_t hash = Fnv::OffsetBasis;
        for (char octet : octets)
        {
            hash ^= octet;
            hash *= Fnv::Prime;
        }

        return hash;
    }

}