#include "pch.h"
#include "DodoEngine.h"

////////////////////////////////////////////////////////////////
// SANDBOX /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

class Sandbox : public Dodo::Application
{
public:
    Sandbox(const Dodo::ApplicationSpecs& specs)
        : Application(specs)
    {}

    void OnInit() override
    {
        LOG_CORE_INFO("Hello World!");
        LOG_CORE_WARNING("Hello World!");
        LOG_CORE_ERROR("Hello World!");
        LOG_CORE_FATAL("Hello World!");
    }
};

Dodo::Application* CreateApplication(const Dodo::CommandLineArgs& cmdLineArgs)
{
    Dodo::RenderSettings renderSettings{};
    renderSettings.RenderDeviceType = Dodo::RenderDeviceType::Vulkan;
    renderSettings.VSyncMode        = Dodo::VSyncMode::Enable;
    renderSettings.BackBufferCount  = 2;

    Dodo::ApplicationSpecs appSpecs{};
    appSpecs.CmdLineArgs    = cmdLineArgs;
    appSpecs.Width          = 1280;
    appSpecs.Height         = 720;
    appSpecs.Title          = "Sandbox";
    appSpecs.ShowFrameRate  = true;
    appSpecs.EnableImGui    = true;
    appSpecs.ThreadPolicy   = Dodo::ThreadPolicy::MultiThreaded;
    appSpecs.RenderSettings = renderSettings;
    return new Sandbox(appSpecs);
}