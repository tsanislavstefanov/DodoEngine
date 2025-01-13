#pragma once

#include <cstdint>

namespace Dodo {

    class RenderHandle {
    public:
        RenderHandle() = default;
        RenderHandle(uint64_t handle)
            : _handle(handle) {}

        inline operator bool() const { return !is_null(); }
        inline bool is_null () const { return _handle == 0; }

        template<typename T>
        T cast_to();

        template<typename T>
        const T cast_to() const;

        inline uint64_t get() { return _handle; }
        inline uint64_t get() const { return _handle; }

    protected:
        uint64_t _handle = 0;
    };

    template<typename T>
    inline T RenderHandle::cast_to() {
        return reinterpret_cast<T>(_handle);
    }

    template<typename T>
    inline const T RenderHandle::cast_to() const {
        return reinterpret_cast<T>(_handle);
    }

#define DODO_DEFINE_RENDER_HANDLE(NAME) \
    struct NAME##Handle : public RenderHandle { \
        static inline NAME##Handle null() { return NAME##Handle(); } \
        NAME##Handle() = default; \
        NAME##Handle(uint64_t handle) : RenderHandle(handle) {} \
        NAME##Handle(void* info) : RenderHandle(reinterpret_cast<uint64_t>(info)) {} \
        NAME##Handle(const NAME##Handle& other) : RenderHandle(other._handle) {} \
        inline NAME##Handle& operator=(const NAME##Handle& other) { \
            _handle = other._handle; \
            return *this; \
        } \
        inline bool operator==(const NAME##Handle& other) const { return _handle == other._handle; } \
        inline bool operator!=(const NAME##Handle& other) const { return !((*this) == other); } \
    }

}
