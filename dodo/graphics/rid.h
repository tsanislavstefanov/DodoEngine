#pragma once

namespace Dodo {

    struct RID
    {
        size_t id = 0;

        RID() = default;
        RID(size_t id) : id(id) {}
        virtual ~RID() = default;
    };

#define DODO_DEFINE_RID(NAME) \
    struct NAME##ID : public RID \
    { \
        NAME##ID() = default; \
        NAME##ID(void* info) : RID(reinterpret_cast<size_t>(info)) {} \
        NAME##ID(const NAME##ID& other) : RID(other.id) {} \
        operator bool() const { return id != 0; } \
        NAME##ID& operator=(const NAME##ID& other) \
        { \
            id = other.id; \
            return *this; \
        } \
        bool operator==(const NAME##ID& other) const { return id == other.id; } \
        bool operator!=(const NAME##ID& other) const { return !((*this) == other); } \
    }

}