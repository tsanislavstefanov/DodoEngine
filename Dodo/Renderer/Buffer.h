#pragma once

#include "RenderHandle.h"

namespace Dodo {

    enum class BufferUsage
    {
        VertexBuffer,
        AutoCount,
        None
    };

    DODO_DEFINE_RENDER_HANDLE(Buffer);

}