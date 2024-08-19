#pragma once

#include <cstdint>

#include "RenderApiType.h"
#include "VSyncMode.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER SETTINGS /////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct RenderSettings
    {
        RenderApiType RenderApiType = RenderApiType::None;
        uint32_t ConcurrentFrameCount = 3;
        VSyncMode VSyncMode = VSyncMode::None;
    };

}