#include "pch.h"
#include "Log.h"
#include "Core/Platform.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // CONSOLE LOGGER //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class ConsoleLogger : public Logger
    {
    public:
        ConsoleLogger() = default;

        void WriteLog(LogLevel level, const std::string &message) override
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
            std::array<std::string, MaxSinkCount> Colors = {
                    "\033[32m", // Debug  : Green.
                    "\033[36m", // Info   : Cyan.
                    "\033[33m", // Warning: Yellow.
                    "\033[31m", // Error  : Red.
                    "\033[35m"  // Fatal  : Magenta.
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

    void Log::DeInit()
    {
        s_CoreLogger.reset();
    }

}