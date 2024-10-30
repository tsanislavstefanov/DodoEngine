#pragma once

namespace Dodo {

    struct RenderHandle
    {
        size_t Handle = 0;

        RenderHandle() = default;
        RenderHandle(size_t handle)
            : Handle(handle)
        {}

        virtual ~RenderHandle() = default;
    };

#define DODO_DEFINE_RENDER_HANDLE(NAME) \
    struct NAME##Handle : public RenderHandle \
    { \
        NAME##Handle() = default; \
        NAME##Handle(void* handle) : RenderHandle(reinterpret_cast<size_t>(handle)) {} \
        NAME##Handle(const NAME##Handle& other) : RenderHandle(other.Handle) {} \
        operator bool() const { return Handle != 0; } \
        NAME##Handle& operator=(const NAME##Handle& other) \
        { \
            Handle = other.Handle; \
            return *this; \
        } \
        bool operator==(const NAME##Handle& other) const { return Handle == other.Handle; } \
        bool operator!=(const NAME##Handle& other) const { return !((*this) == other); } \
    }

}