#pragma once

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // SWAPCHAIN ///////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Swapchain : public RefCounted
    {
    public:
        static Ref<Swapchain> Create(void* windowHandle, uint32_t width, uint32_t height);

        virtual ~Swapchain() = default;

        virtual void BeginFrame() = 0;
        virtual void OnResize(uint32_t width, uint32_t height) = 0;
        virtual void Present() = 0;
        virtual void Destroy() = 0;
    };

}