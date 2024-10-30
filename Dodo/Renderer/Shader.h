#pragma once

#include "RenderHandle.h"

namespace Dodo {

    struct ShaderStageMetadata
    {
        size_t Hash = 0;

        ShaderStageMetadata() = default;
        ShaderStageMetadata(size_t hash) : Hash(hash) {}

        bool operator==(const ShaderStageMetadata& other) const { return  Hash == other.Hash; }
        bool operator!=(const ShaderStageMetadata& other) const { return !((*this) == other); }
    };

    DODO_DEFINE_RENDER_HANDLE(Shader);

    enum class ShaderInputType
    {
        Int, Int2, Int3, Int4,
        Float, Float2, Float3, Float4,
        Mat3, Mat4,
        AutoCount,
        None
    };

    namespace Utils {

        static size_t GetShaderInputTypeSize(ShaderInputType type)
        {
            switch (type)
            {
                case ShaderInputType::Int:    return sizeof(int);
                case ShaderInputType::Int2:   return sizeof(int) * 2;
                case ShaderInputType::Int3:   return sizeof(int) * 3;
                case ShaderInputType::Int4:   return sizeof(int) * 4;
                case ShaderInputType::Float:  return sizeof(float);
                case ShaderInputType::Float2: return sizeof(float) * 2;
                case ShaderInputType::Float3: return sizeof(float) * 3;
                case ShaderInputType::Float4: return sizeof(float) * 4;
                case ShaderInputType::Mat3:   return sizeof(float) * 3 * 3;
                case ShaderInputType::Mat4:   return sizeof(float) * 4 * 4;
                default: break;
            }

            DODO_ASSERT(false, "ShaderInputType {0} is not supported!", static_cast<uint32_t>(type));
            return 0;
        }

    }

    struct ShaderInputAttrib
    {
        ShaderInputType Type = ShaderInputType::None;
        std::string Name{};
        size_t Size = 0;
        size_t Offset = 0;
        bool Normalized = false;

        ShaderInputAttrib() = default;
        ShaderInputAttrib(ShaderInputType type, const std::string& name, bool normalized = false)
            : Type(type),
              Name(name),
              Size(Utils::GetShaderInputTypeSize(Type)),
              Normalized(normalized)
        {
        }
    };

    class ShaderInputAttribBinding
    {
    public:
        ShaderInputAttribBinding(const std::initializer_list<ShaderInputAttrib>& attribs)
            : m_Attribs(attribs)
        {
            CalculateOffsetsAndStride();
        }

        std::vector<ShaderInputAttrib>::iterator begin() { return m_Attribs.begin(); }
        std::vector<ShaderInputAttrib>::iterator end() { return m_Attribs.end(); }
        std::vector<ShaderInputAttrib>::const_iterator begin() const { return m_Attribs.begin(); }
        std::vector<ShaderInputAttrib>::const_iterator end() const { return m_Attribs.end(); }

        size_t GetStride() const { return m_Stride; }
        const std::vector<ShaderInputAttrib>& GetAttribs() const { return m_Attribs; }
        uint32_t GetAttribCount() const { return static_cast<uint32_t>(m_Attribs.size()); }

    private:
        void CalculateOffsetsAndStride();

        std::vector<ShaderInputAttrib> m_Attribs{};
        size_t m_Stride = 0;
    };

}