#include "pch.h"
#include "DodoEngine.h"

////////////////////////////////////////////////////////////////
// SANDBOX /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

class Sandbox : public Application
{
public:
    Sandbox(const ApplicationSpecs& specs)
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

Application* CreateApplication(const CommandLineArgs& cmdLineArgs)
{
    RenderSettings renderSettings{};
    renderSettings.RenderDeviceType = RenderDeviceType::Vulkan;
    renderSettings.VSyncMode        = VSyncMode::Enable;
    renderSettings.BackBufferCount  = 2;

    ApplicationSpecs appSpecs{};
    appSpecs.CmdLineArgs    = cmdLineArgs;
    appSpecs.Width          = 1280;
    appSpecs.Height         = 720;
    appSpecs.Title          = "Sandbox";
    appSpecs.ShowFrameRate  = true;
    appSpecs.EnableImGui    = false;
    appSpecs.RenderSettings = renderSettings;
    return new Sandbox(appSpecs);
}