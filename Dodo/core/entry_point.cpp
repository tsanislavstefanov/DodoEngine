#pragma once

#include "engine.h"
#include "diagnostics/log.h"

////////////////////////////////////////////////////////////////////
// ENTRY POINT /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    Dodo::Log::init();

    Dodo::Engine engine({ argc, argv });
    engine.iterate_main_loop();
    return 0;
}