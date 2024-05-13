#include "pch.h"
#include "log.h"
#include "core/platform.h"

////////////////////////////////////////////////////////////////
// CONSOLE LOGGER //////////////////////////////////////////////
////////////////////////////////////////////////////////////////

class ConsoleLogger : public Logger
{
public:
    ConsoleLogger() = default;

    void WriteLog(LogLevel level, const std::string& message) override
    {
        std::string dateTime  = Platform::GetDatetimeAsString();
        const auto  sinkIndex = static_cast<size_t>(level);
        std::cout << std::format("{0}: {1}{2}{3}\n",
                                 dateTime,
                                 m_Sink.Colors.at(sinkIndex),
                                 message,
                                 m_Sink.Off);
    }

private:
    ////////////////////////////////////////////////////////////
    // SINK ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////

    struct Sink
    {
        static constexpr auto MaxSinkCount = static_cast<size_t>(LogLevel::AutoCount);
        std::array<std::string, MaxSinkCount> Colors =
        {
            "\033[32m", // debug  : green
            "\033[36m", // info   : cyan
            "\033[33m", // warning: yellow
            "\033[31m", // error  : red
            "\033[35m"  // fatal  : magenta
        };
        std::string Off = "\033[0m";
    };

    Sink m_Sink{};
};

////////////////////////////////////////////////////////////////
// LOGGER //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

std::shared_ptr<Logger> Logger::Create(LoggerType type)
{
    switch (type)
    {
        case LoggerType::Console: return std::make_shared<ConsoleLogger>();
        default:                  return nullptr;
    }
}

////////////////////////////////////////////////////////////////
// LOG /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

std::shared_ptr<Logger> Log::s_CoreLogger = nullptr;

void Log::Init()
{
    s_CoreLogger = Logger::Create(LoggerType::Console);
}

void Log::Deinit()
{
    s_CoreLogger.reset();
}