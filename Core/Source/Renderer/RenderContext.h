#pragma once

#include "RenderDeviceType.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // FORWARD DECLARATIONS ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class RenderDevice;
    class Swapchain;

    ////////////////////////////////////////////////////////////////
    // RENDER CONTEXT //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class RenderContext : public RefCounted
    {
    public:
        static Ref<RenderContext> Create(RenderDeviceType renderDeviceType);

        virtual ~RenderContext() = default;

        virtual Ref<RenderDevice> CreateRenderDevice() const = 0;

        virtual void Destroy() = 0;

        virtual Ref<Swapchain> GetSwapchain() const = 0;
    };

}