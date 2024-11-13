#pragma once

namespace Dodo {

    struct RenderHandle
    {
        size_t handle = 0;

        RenderHandle() = default;
        RenderHandle(size_t handle)
            : handle{handle}
        {}

        virtual ~RenderHandle() = default;
    };

#define DODO_DEFINE_RENDER_HANDLE(NAME) \
    struct NAME##Handle : public RenderHandle \
    { \
        NAME##Handle() = default; \
        NAME##Handle(void* handle) : RenderHandle(reinterpret_cast<size_t>(handle)) {} \
        NAME##Handle(const NAME##Handle& other) : RenderHandle(other.handle) {} \
        operator bool() const { return handle != 0; } \
        NAME##Handle& operator=(const NAME##Handle& other) { \
            handle = other.handle; \
            return *this; \
        } \
        bool operator==(const NAME##Handle& other) const { return handle == other.handle; } \
        bool operator!=(const NAME##Handle& other) const { return !((*this) == other); } \
    }

}