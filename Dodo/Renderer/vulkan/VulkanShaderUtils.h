#pragma once

#include "VulkanUtils.h"
#include "Core/Core.h"

#include <string>
#include <shaderc/shaderc.hpp>

namespace Dodo {

    namespace Utils {

        static std::string ConvertShaderStageToString(VkShaderStageFlagBits shaderStage)
        {
            switch (shaderStage)
            {
                case VK_SHADER_STAGE_VERTEX_BIT:   return "Vertex";
                case VK_SHADER_STAGE_FRAGMENT_BIT: return "Fragment";
                default: break;
            }

            DODO_VERIFY(false);
            return "None";
        }

    }

}