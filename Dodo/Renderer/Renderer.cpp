#include "pch.h"
#include "renderer.h"
#include "Drivers/Vulkan/VulkanRenderer.h"

namespace Dodo {

    Ref<Renderer> Renderer::create(const Specification& specification)
    {
        switch (specification.type)
        {
            case Type::vulkan: return Ref<VulkanRenderer>::create(specification);
            default: break;
        }

        DODO_ASSERT(false, "Renderer type not supported!");
        return nullptr;
    }

}