#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include "Renderer/Shader.h"

namespace Dodo {

    struct VulkanShader
    {
        std::filesystem::path AssetPath{};
    };

}