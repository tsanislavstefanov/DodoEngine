#pragma once

#include <cstdint>

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER API ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class RendererApi
    {
    public:
        virtual ~RendererApi() = default;

        virtual void Shutdown() = 0;
    };

}