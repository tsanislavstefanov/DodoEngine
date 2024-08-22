#pragma once

#include <spdlog/spdlog.h>

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // LOG /////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Log
    {
    public:
        ////////////////////////////////////////////////////////////
        // TYPE ////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        enum class Type
        {
            Core, // Core engine.
            Client, // Runtime client (or sandbox).
            Both,
            AutoCount,
            None
        };

        ////////////////////////////////////////////////////////////
        // LEVEL ///////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        enum class Level
        {
            Trace,
            Info,
            Warning,
            Error,
            Fatal,
            AutoCount,
            None
        };

        ////////////////////////////////////////////////////////////
        // TAG INFO ////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct TagInfo
        {
            bool Enabled = true;
            Type TypeFilter = Type::Both;
            Level LevelFilter = Level::Trace;
        };

        static void Init();

        static void DeInit();

        static std::shared_ptr<spdlog::logger> GetCoreLogger()
        {
            return s_CoreLogger;
        }

        static std::shared_ptr<spdlog::logger> GetClientLogger()
        {
            return s_ClientLogger;
        }

    private:
        template<typename... Args>
        void WriteMessage(Type type, Level level, std::format_string<Args...> fmt, Args&&... args)
        {

        }

        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
    };

}

#ifdef DODO_DEBUG
#   define LOG_CORE_DEBUG(...)   Dodo::Log::GetCoreLogger()->trace   (__VA_ARGS__)
#   define LOG_CORE_INFO(...)    Dodo::Log::GetCoreLogger()->info    (__VA_ARGS__)
#   define LOG_CORE_WARNING(...) Dodo::Log::GetCoreLogger()->warn    (__VA_ARGS__)
#   define LOG_CORE_ERROR(...)   Dodo::Log::GetCoreLogger()->error   (__VA_ARGS__)
#   define LOG_CORE_FATAL(...)   Dodo::Log::GetCoreLogger()->critical(__VA_ARGS__)
#else
#   define LOG_CORE_DEBUG(...)
#   define LOG_CORE_INFO(...)
#   define LOG_CORE_WARNING(...)
#   define LOG_CORE_ERROR(...)
#   define LOG_CORE_FATAL(...)
#endif