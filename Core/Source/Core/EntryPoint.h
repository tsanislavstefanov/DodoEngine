#pragma once

#include "Application.h"
#include "Diagnostics/Log.h"

extern Application* CreateApplication(const CommandLineArgs& cmdLineArgs);

int main(int argc, char** argv)
{
    Log::Init();

    Application* application = CreateApplication({argc, argv });
    application->Run();
    delete application;

    Log::DeInit();
    return 0;
}