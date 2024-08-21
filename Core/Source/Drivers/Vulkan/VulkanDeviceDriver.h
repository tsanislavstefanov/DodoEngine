#pragma once

#include "Renderer/RenderDeviceDriver.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN DEVICE DRIVER ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanDeviceDriver : public RenderDeviceDriver
    {
    public:
        VulkanDeviceDriver() = default;

        // Inherited via [RenderDeviceDriver].
        void Destroy() override;
    };

}