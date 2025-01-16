#pragma once

#include <cstdint>

namespace Dodo {

    class RenderHandle {
    public:
        RenderHandle() = default;
        RenderHandle(uint64_t id) : _id(id) {}

        inline bool is_null() const {
            return _id == 0;
        }

        inline uint64_t get_id() const {
            return _id;
        }

    protected:
        uint64_t _id = 0;
    };

#define DODO_DEFINE_RENDER_HANDLE(NAME) \
    class NAME##Handle : public RenderHandle { \
    public: \
        NAME##Handle() = default; \
        NAME##Handle(uint64_t id) : RenderHandle(id) {} \
        NAME##Handle(const NAME##Handle& other) : RenderHandle(other.get_id()) {} \
        inline NAME##Handle& operator=(const NAME##Handle& other) { \
            _id = other.get_id(); \
            return *this; \
        } \
        inline bool operator==(const NAME##Handle& other) const { \
            return _id == other.get_id(); \
        } \
        inline bool operator!=(const NAME##Handle& other) const { \
            return !((*this) == other); \
        } \
    }

}
