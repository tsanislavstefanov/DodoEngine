#pragma once

#include <cstdint>

#include "RenderDeviceType.h"
#include "VSyncMode.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER SETTINGS /////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct RenderSettings
    {
        RenderDeviceType RenderDeviceType = RenderDeviceType::None;
        uint32_t ConcurrentFrameCount = 3;
        VSyncMode VSyncMode = VSyncMode::None;
    };

}