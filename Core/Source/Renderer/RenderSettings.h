#pragma once

#include <cstdint>

#include "RendererApiType.h"
#include "VSyncMode.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER SETTINGS /////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct RenderSettings
    {
        RendererApiType RendererApiType = RendererApiType::None;
        VSyncMode VSyncMode = VSyncMode::None;
        uint32_t FramesInFlight = 2;
    };

}