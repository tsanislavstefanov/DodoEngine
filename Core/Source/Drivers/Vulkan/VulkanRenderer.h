#pragma once

#include "Renderer/RendererApi.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN RENDERER /////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanRenderer : public RendererApi
    {
    public:
        VulkanRenderer() = default;

        // Inherited via [RendererApi].
        void Shutdown() override;
    };

}