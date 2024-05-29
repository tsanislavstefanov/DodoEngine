#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "Core/Core.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // BUFFER //////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct Buffer
    {
        static Buffer Copy(void* data, size_t size);
        static void CopyTo(Buffer destination, void* data, size_t size);

        void*  Data;
        size_t Size;

        Buffer()
            : Data(nullptr)
            , Size(0)
        {}

        Buffer(std::nullptr_t)
            : Data(nullptr)
            , Size(0)
        {}

        Buffer(size_t size)
            : Data(new uint8_t[size])
            , Size(size)
        {}

        Buffer(void* data, size_t size)
            : Data(data)
            , Size(size)
        {}

        operator bool() const { return (Data != nullptr) && (Size > 0); }

        void Allocate(size_t size);

        template<typename T>
        T* As() { return (T*)Data; }

        template<typename T>
        T* As() const { return (T*)Data; }

        void Release();
    };

}