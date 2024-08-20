#pragma once

#include <cstdint>

#include "Memory/Ref.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // SWAPCHAIN ///////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Swapchain : public RefCounted
    {
    public:
        virtual ~Swapchain() = default;

        virtual void BeginFrame() = 0;

        virtual void Present() = 0;

        virtual void Destroy() = 0;
        
        virtual void OnResize(uint32_t width, uint32_t height) = 0;
    };

}