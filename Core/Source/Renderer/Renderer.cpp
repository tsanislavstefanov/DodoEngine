#include "pch.h"
#include "Renderer.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDERER ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    static Renderer::Data s_Data{};

    void Renderer::Init(const RendererSpecs& specs)
    {
        s_Data.Specs = specs;
        s_Data.Context = RenderContext::Create(s_Data.Specs.DeviceDriverType);
        s_Data.DeviceDriver = s_Data.Context->CreateDeviceDriver();
    }

    void Renderer::Destroy()
    {
        s_Data.DeviceDriver->Destroy();
        s_Data.Context->Destroy();
    }

}