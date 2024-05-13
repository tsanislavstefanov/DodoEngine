#pragma once

#include <cstdint>

#include "RenderDeviceType.h"
#include "VSyncMode.h"

////////////////////////////////////////////////////////////////
// RENDER SETTINGS /////////////////////////////////////////////
////////////////////////////////////////////////////////////////

struct RenderSettings
{
    RenderDeviceType RenderDeviceType = RenderDeviceType::None;
    VSyncMode        VSyncMode        = VSyncMode::None;
    uint32_t         BackBufferCount  = 2;
};