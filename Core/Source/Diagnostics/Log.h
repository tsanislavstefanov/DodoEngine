#pragma once

////////////////////////////////////////////////////////////////
// LOGGER TYPE /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

enum class LoggerType
{
    Console  ,
    AutoCount,
    None
};

////////////////////////////////////////////////////////////////
// LOG LEVEL ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

enum class LogLevel
{
    Debug    ,
    Info     ,
    Warning  ,
    Error    ,
    Fatal    ,
    AutoCount,
    None
};

////////////////////////////////////////////////////////////////
// LOGGER //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

class Logger
{
public:
    static std::shared_ptr<Logger> Create(LoggerType type);

    Logger() = default;
    virtual ~Logger() = default;

    template<typename... Args>
    void LogDebug(const std::string& format, Args&&... args)
    {
        std::string message = std::vformat(format, std::make_format_args(args...));
        WriteLog(LogLevel::Debug, message);
    }

    template<typename... Args>
    void LogInfo(const std::string& format, Args&&... args)
    {
        std::string message = std::vformat(format, std::make_format_args(args...));
        WriteLog(LogLevel::Info, message);
    }

    template<typename... Args>
    void LogWarning(const std::string& format, Args&&... args)
    {
        std::string message = std::vformat(format, std::make_format_args(args...));
        WriteLog(LogLevel::Warning, message);
    }

    template<typename... Args>
    void LogError(const std::string& format, Args&&... args)
    {
        std::string message = std::vformat(format, std::make_format_args(args...));
        WriteLog(LogLevel::Error, message);
    }

    template<typename... Args>
    void LogFatal(const std::string& format, Args&&... args)
    {
        std::string message = std::vformat(format, std::make_format_args(args...));
        WriteLog(LogLevel::Fatal, message);
    }

protected:
    virtual void WriteLog(LogLevel level, const std::string& message) = 0;
};

////////////////////////////////////////////////////////////////
// LOG /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

class Log
{
public:
    static void Init();

    static std::shared_ptr<Logger> GetCoreLogger()
    {
        return s_CoreLogger;
    }

    static void DeInit();

private:
    static std::shared_ptr<Logger> s_CoreLogger;
};

#ifdef CONFIGURATION_DEBUG
#   define LOG_CORE_DEBUG(...)   Log::GetCoreLogger()->LogDebug  (__VA_ARGS__)
#   define LOG_CORE_INFO(...)    Log::GetCoreLogger()->LogInfo   (__VA_ARGS__)
#   define LOG_CORE_WARNING(...) Log::GetCoreLogger()->LogWarning(__VA_ARGS__)
#   define LOG_CORE_ERROR(...)   Log::GetCoreLogger()->LogError  (__VA_ARGS__)
#   define LOG_CORE_FATAL(...)   Log::GetCoreLogger()->LogFatal  (__VA_ARGS__)
#else
#   define LOG_CORE_DEBUG(...)
#   define LOG_CORE_INFO(...)
#   define LOG_CORE_WARNING(...)
#   define LOG_CORE_ERROR(...)
#   define LOG_CORE_FATAL(...)
#endif