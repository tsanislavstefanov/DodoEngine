#pragma once

#include "Application.h"
#include "Diagnostics/Log.h"

extern Dodo::Application* CreateApplication(const Dodo::CommandLineArgs& cmdLineArgs);

int main(int argc, char** argv)
{
    Dodo::Log::Init();

    Dodo::Application* application = CreateApplication({argc, argv });
    application->Run();
    delete application;

    Dodo::Log::DeInit();
    return 0;
}