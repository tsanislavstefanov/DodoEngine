#include "pch.h"
#include "renderer.h"
#include "Drivers/Vulkan/vulkan_renderer.h"

namespace Dodo {

    Ref<Renderer> Renderer::create(Type type)
    {
        switch (type)
        {
            case Type::vulkan: return Ref<RendererVulkan>::create();
            default: break;
        }

        DODO_ASSERT(false, "Renderer type not supported!");
        return nullptr;
    }

}