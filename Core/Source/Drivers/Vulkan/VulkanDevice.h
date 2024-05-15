#pragma once

#include "VulkanContext.h"
#include "Renderer/RenderDevice.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN DEVICE ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanDevice : public RenderDevice
    {
    public:
        VulkanDevice();

        // Inherited via [RenderDevice].
        void BeginFrame() override;

        void Resize(uint32_t width, uint32_t height) override;

        void EndFrame() override;

        void Dispose() override;

    private:
        std::unique_ptr<VulkanContext> m_Context = nullptr;
    };

}