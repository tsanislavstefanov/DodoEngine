#pragma once

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // SWAPCHAIN ///////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Swapchain : public RefCounted
    {
    public:
        static Ref<Swapchain> Create();

        virtual ~Swapchain() = default;

        virtual void BeginFrame_RenderThread() = 0;
        virtual void OnResize_RenderThread(uint32_t width, uint32_t height) = 0;
        virtual void Present_RenderThread() = 0;
        virtual void Destroy() = 0;
    };

}