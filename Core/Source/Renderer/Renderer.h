#pragma once

#include "RenderContext.h"
#include "RenderDeviceDriver.h"
#include "VSyncMode.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDERER SPECS //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct RendererSpecs
    {
        RenderDeviceDriverType DeviceDriverType = RenderDeviceDriverType::None;
        uint32_t ConcurrentFrameCount = 3;
        VSyncMode VSyncMode = VSyncMode::None;
    };

    ////////////////////////////////////////////////////////////////
    // RENDERER ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Renderer
    {
    public:
        static void Init(const RendererSpecs& specs);

        static void Destroy();

        static const RendererSpecs& GetSpecs()
        {
            return s_Data.Specs;
        }

        static Ref<RenderContext> GetContext()
        {
            return s_Data.Context;
        }

    private:

        ////////////////////////////////////////////////////////////
        // DATA ////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct Data
        {
            RendererSpecs Specs{};
            Ref<RenderContext> Context = nullptr;
            Ref<RenderDeviceDriver> DeviceDriver = nullptr;
        };

        static Data s_Data;
    };

}