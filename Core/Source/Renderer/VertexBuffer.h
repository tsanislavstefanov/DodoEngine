#pragma once

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN VERTEX BUFFER ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VertexBuffer : public RefCounted
    {
    public:
        Ref<VertexBuffer> Create(size_t size);
        Ref<VertexBuffer> Create(void* data, size_t size);

        virtual ~VertexBuffer() = default;
    };

}