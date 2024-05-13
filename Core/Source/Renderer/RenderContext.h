#pragma once

////////////////////////////////////////////////////////////////
// RENDER CONTEXT //////////////////////////////////////////////
////////////////////////////////////////////////////////////////

class RenderContext : public RefCounted
{
public:
    virtual ~RenderContext() = default;

    virtual void PrepareBuffers() = 0;

    virtual void Resize(uint32_t width, uint32_t height) = 0;

    virtual void SwapBuffers() = 0;

    virtual void Dispose() = 0;
};