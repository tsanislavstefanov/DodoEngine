#include "pch.h"
#include "Platform.h"

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

    void Platform::SetThreadName(std::thread& thread, const std::string& name)
    {
#ifdef DODO_WINDOWS
        const std::wstring temp(name.begin(), name.end());
        SetThreadDescription(reinterpret_cast<HANDLE>(thread.native_handle()), temp.c_str());
#endif
    }

}