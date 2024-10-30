#include "pch.h"
#include "VulkanUtils.h"
#include "VulkanShaderCache.h"

namespace Dodo {

    void VulkanShaderCache::Serialize()
    {
    }

    void VulkanShaderCache::Deserialize()
    {
    }

    VkShaderStageFlagBits VulkanShaderCache::GetChangedStages(VulkanShader* shader)
    {
        return VkShaderStageFlagBits();
    }

}