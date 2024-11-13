#pragma once

#include "render_handle.h"

namespace Dodo {

    enum class BufferUsage
    {
        VertexBuffer,
        AutoCount,
        None
    };

    DODO_DEFINE_RENDER_HANDLE(Buffer);

}