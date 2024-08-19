#include "pch.h"
#include "ByteBuffer.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // BYTE BUFFER /////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    ByteBuffer ByteBuffer::Copy(void* data, size_t size)
    {
        ByteBuffer result = nullptr;
        result.Allocate(size);
        std::memcpy(result.Data, data, result.Size);
        return result;
    }

    void ByteBuffer::CopyTo(ByteBuffer destination, void* data, size_t size)
    {
        DODO_ASSERT(size <= destination.Size, "!");
        std::memcpy(destination.Data, data, size);
    }

    void ByteBuffer::Allocate(size_t size)
    {
        Release();
        Data = new uint8_t[size];
        Size = size;
    }

    void ByteBuffer::Release()
    {
        if (Data)
        {
            delete[](uint8_t*)Data;
        }

        Data = nullptr;
        Size = 0;
    }

}