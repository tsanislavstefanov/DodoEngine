#include "pch.h"
#include "Shader.h"

namespace Dodo {

    void ShaderInputAttribBinding::CalculateOffsetsAndStride()
    {
        size_t offset = 0;
        m_Stride = 0;
        for (ShaderInputAttrib& attrib : m_Attribs)
        {
            attrib.Offset = offset;
            offset += attrib.Size;
            m_Stride += attrib.Size;
        }
    }

}