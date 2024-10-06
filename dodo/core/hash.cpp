#include "pch.h"
#include "hash.h"

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
        static constexpr uint32_t prime = 16777619;
        static constexpr uint32_t offset_basis = 2166136261;
    };

    template<>
    struct FnvTraits<uint64_t>
    {
        static constexpr uint64_t prime = 1099511628211;
        static constexpr uint64_t offset_basis = 14695981039346656037;
    };

    ////////////////////////////////////////////////////////////////
    // HASH ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    size_t Hash::generate_fnv1a(const std::string& octets)
    {
        // Reference for the algorithm and the values for
        // prime and offset basis online: http://www.isthe.com/chongo/tech/comp/fnv/.
        using Fnv = FnvTraits<size_t>;
        size_t hash = Fnv::offset_basis;
        for (char octet : octets)
        {
            hash ^= octet;
            hash *= Fnv::prime;
        }

        return hash;
    }

}