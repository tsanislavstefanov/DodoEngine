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
        VSyncMode        VSyncMode        = VSyncMode::None;
        uint32_t         BackBufferCount  = 2;
    };

}