#pragma once

#include "Renderer/Shader.h"

namespace Dodo {

    struct VulkanShader
    {
        std::filesystem::path AssetPath{};
        std::string Source{};
        ShaderStageMetadata Metadata{};
        std::vector<uint32_t> SPIRVData{};
    };

}