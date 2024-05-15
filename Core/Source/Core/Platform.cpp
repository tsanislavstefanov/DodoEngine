#include "pch.h"
#include "platform.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // PLATFORM ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    std::string Platform::GetDatetimeAsString()
    {
        using LocalTime = std::chrono::local_time<std::chrono::system_clock::duration>;
        LocalTime localTime = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        return std::format("{:%d/%m/%Y %H:%M:%S}", localTime);
    }

}