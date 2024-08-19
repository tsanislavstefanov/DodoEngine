#pragma once

#include "Renderer/RenderApi.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN RENDER API ///////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanRenderApi : public RenderApi
    {
    public:
        VulkanRenderApi() = default;

        // Inherited via [RenderApi].
        void Destroy() override;
    };

}