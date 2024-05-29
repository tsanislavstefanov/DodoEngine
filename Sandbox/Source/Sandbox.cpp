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
            :
            Application(specs)
        {}

    };

}

Dodo::Application* CreateApplication(const Dodo::CommandLineArgs& cmdLineArgs)
{
    Dodo::RenderSettings renderSettings{};
    renderSettings.RendererApiType = Dodo::RendererApiType::Vulkan;
    renderSettings.VSyncMode = Dodo::VSyncMode::Disable;
    renderSettings.FramesInFlight = 2;

    Dodo::ApplicationSpecs appSpecs{};
    appSpecs.CmdLineArgs = cmdLineArgs;
    appSpecs.Width = 1280;
    appSpecs.Height = 720;
    appSpecs.Title = "Sandbox";
    appSpecs.ShowFrameRate = true;
    appSpecs.EnableImGui = false;
    appSpecs.ThreadingPolicy = Dodo::ThreadingPolicy::MultiThreaded;
    appSpecs.RenderSettings = renderSettings;
    return new Dodo::Sandbox(appSpecs);
}