#pragma once

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // BYTE BUFFER /////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct ByteBuffer
    {
        static ByteBuffer Copy(void* data, size_t size);
        static void CopyTo(ByteBuffer destination, void* data, size_t size);

        void*  Data;
        size_t Size;

        ByteBuffer()
            : Data(nullptr)
            , Size(0)
        {}

        ByteBuffer(std::nullptr_t)
            : Data(nullptr)
            , Size(0)
        {}

        ByteBuffer(size_t size)
            : Data(new uint8_t[size])
            , Size(size)
        {}

        ByteBuffer(void* data, size_t size)
            : Data(data)
            , Size(size)
        {}

        operator bool() const
        {
            return (Data != nullptr) && (Size > 0);
        }

        void Allocate(size_t size);

        template<typename T>
        T* As()
        {
            return static_cast<T*>(Data);
        }

        template<typename T>
        T* As() const
        {
            return static_cast<T*>(Data);
        }

        void Release();
    };

}