#pragma once

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // PLATFORM ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Platform
    {
    public:
        static void SetThreadName(std::thread& thread, const std::string& name);
        static void SetThreadAffinity(std::thread& thread, uint64_t affinity);
        static std::string GetDatetimeAsString();
    };

}