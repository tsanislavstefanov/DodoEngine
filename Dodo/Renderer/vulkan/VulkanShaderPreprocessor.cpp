#include "pch.h"
#include "vulkan_utils.h"
#include "VulkanShaderPreprocessor.h"

namespace Dodo {

    namespace Utils {

        static VkShaderStageFlagBits GetShaderStageFromString(const std::string& stageName)
        {
            if (stageName == "vertex")
                return VK_SHADER_STAGE_VERTEX_BIT;
            else if (stageName == "fragment")
                return VK_SHADER_STAGE_FRAGMENT_BIT;

            DODO_VERIFY(false);
            return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        }

    }

    std::map<VkShaderStageFlagBits, std::string> VulkanShaderPreprocessor::SplitToStages(const std::string& shaderSource)
    {
        std::map<VkShaderStageFlagBits, std::string> shaderStages{};
        const std::string token = "#pragma stage:";
        size_t pos = shaderSource.find(token, 0);
        while (pos != std::string::npos)
        {
            const size_t eol = shaderSource.find_first_of("\r\n", pos);
            if (eol == std::string::npos)
                return {};

            const size_t start = pos + token.length() + 1;
            const VkShaderStageFlagBits shaderStage = Utils::GetShaderStageFromString(shaderSource.substr(start, eol - start));
            size_t nextLinePos = shaderSource.find_first_not_of("\r\n", eol);
            if (nextLinePos == std::string::npos)
                return {};

            pos = shaderSource.find(token, nextLinePos);
            shaderStages[shaderStage] = (pos == std::string::npos) ? shaderSource.substr(nextLinePos) : shaderSource.substr(nextLinePos, pos - nextLinePos);
        }

        return shaderStages;
    }

}