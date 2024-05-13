#pragma once

#include <cstdint>

////////////////////////////////////////////////////////////////
// RENDER DEVICE ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////

class RenderDevice
{
public:
    virtual ~RenderDevice() = default;

    virtual void BeginFrame() = 0;

    virtual void Resize(uint32_t width, uint32_t height) = 0;

    virtual void EndFrame() = 0;

    virtual void Dispose() = 0;
};