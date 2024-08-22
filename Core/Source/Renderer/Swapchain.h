#pragma once

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // SWAPCHAIN ///////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Swapchain : public RefCounted
    {
    public:
        static Ref<Swapchain> Create(void* windowHandle, uint32_t windowWidth, uint32_t windowHeight);

        virtual ~Swapchain() = default;

        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void ResizeViewport(uint32_t width, uint32_t height) = 0;
        virtual void Destroy() = 0;
    };

}