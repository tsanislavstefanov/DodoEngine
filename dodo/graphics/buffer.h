#pragma once

#include <cstdint>

#include "buffer_usage.h"
#include "rid.h"

namespace Dodo {

    struct BufferSpecifications
    {
        BufferUsage usage = BufferUsage::none;
        size_t size = 0;
        void* initial_data = nullptr;
    };

    DODO_DEFINE_RID(Buffer);

}