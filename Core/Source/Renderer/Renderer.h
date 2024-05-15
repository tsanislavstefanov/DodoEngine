#pragma once

#include "RenderSettings.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDERER ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Renderer
    {
    public:
        static const RenderSettings& GetSettings();

        static void SetSettings(const RenderSettings& settings);

        static void Init();

        static void BeginFrame();

        static void Resize(uint32_t width, uint32_t height);

        static void EndFrame();

        static void Dispose();
    };

}