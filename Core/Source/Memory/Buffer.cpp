#include "pch.h"
#include "Buffer.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // BUFFER //////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Buffer Buffer::Copy(void* data, size_t size)
    {
        Buffer result = nullptr;
        result.Allocate(size);
        std::memcpy(result.Data, data, result.Size);
        return result;
    }

    void Buffer::CopyTo(Buffer destination, void* data, size_t size)
    {
        DODO_VERIFY(size <= destination.Size);
        std::memcpy(destination.Data, data, size);
    }

    void Buffer::Allocate(size_t size)
    {
        Release();
        Data = new uint8_t[size];
        Size = size;
    }

    void Buffer::Release()
    {
        if (Data)
        {
            delete[](uint8_t*)Data;
        }

        Data = nullptr;
        Size = 0;
    }

}