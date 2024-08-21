#pragma once

#include "RenderDeviceDriver.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // FORWARD DECLARATIONS ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Swapchain;

    ////////////////////////////////////////////////////////////////
    // RENDER CONTEXT //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class RenderContext : public RefCounted
    {
    public:
        static Ref<RenderContext> Create(RenderDeviceDriverType deviceDriverType);

        virtual ~RenderContext() = default;

        virtual Ref<RenderDeviceDriver> CreateDeviceDriver() const = 0;

        virtual void Destroy() = 0;

        virtual Ref<Swapchain> GetSwapchain() const = 0;
    };

}