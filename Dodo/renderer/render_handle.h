#pragma once

namespace Dodo {

    struct RenderHandle {
        size_t handle = 0;

        RenderHandle(size_t handle)
            : handle{handle} {}

        operator bool() const;

        template<typename T>
        T cast_to();

        template<typename T>
        const T cast_to() const;
    };

    template<typename T>
    inline T RenderHandle::cast_to() {
        return reinterpret_cast<T>(handle);
    }

    template<typename T>
    inline const T RenderHandle::cast_to() const {
        return reinterpret_cast<T>(handle);
    }

#define DODO_DEFINE_RENDER_HANDLE(NAME) \
    struct NAME##Handle : public RenderHandle \
    { \
        NAME##Handle() : NAME##Handle(nullptr) {} \
        NAME##Handle(size_t handle) : RenderHandle(handle) {} \
        NAME##Handle(void* data) : RenderHandle(reinterpret_cast<size_t>(data)) {} \
        NAME##Handle(const NAME##Handle& other) : RenderHandle(other.handle) {} \
        NAME##Handle& operator=(const NAME##Handle& other) { \
            handle = other.handle; \
            return *this; \
        } \
        bool operator==(const NAME##Handle& other) const { return handle == other.handle; } \
        bool operator!=(const NAME##Handle& other) const { return !((*this) == other); } \
    }

}
