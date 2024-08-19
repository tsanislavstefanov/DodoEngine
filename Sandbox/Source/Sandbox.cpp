#include "pch.h"
#include "DodoEngine.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // SANDBOX /////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Sandbox : public Dodo::Application
    {
    public:
        Sandbox(const Dodo::ApplicationSpecs& specs)
            : Application(specs)
        {}

    };

}

////////////////////////////////////////////////////////////////////
// EXTERN DEFINITION(S) ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

Dodo::Application* CreateApplication(const Dodo::CommandLineArgs& cmdLineArgs)
{
    Dodo::RenderSettings renderSettings{};
    renderSettings.RenderApiType = Dodo::RenderApiType::Vulkan;
    renderSettings.ConcurrentFrameCount = 3;
    renderSettings.VSyncMode = Dodo::VSyncMode::Disable;

    Dodo::ApplicationSpecs appSpecs{};
    appSpecs.CmdLineArgs = cmdLineArgs;
    appSpecs.Width = 1280;
    appSpecs.Height = 720;
    appSpecs.Title = "Sandbox";
    appSpecs.EnableImGui = true;
    appSpecs.ThreadPolicy = Dodo::ThreadPolicy::MultiThreaded;
    appSpecs.RenderSettings = renderSettings;
    return new Dodo::Sandbox(appSpecs);
}