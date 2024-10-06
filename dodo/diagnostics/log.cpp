#include "pch.h"
#include "log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // LOG /////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    static Log::TagDetailsMap s_default_tag_settings
    {
        { "RenderDevice", Log::TagDetails{ true, Log::Level::trace }},
    };

    void Log::init()
    {
        s_logger = spdlog::stdout_color_mt("Dodo");
        s_logger->set_level(spdlog::level::trace);
        s_logger->set_pattern("[%T.%e]: %^%v%$");

        use_default_tag_settings();
    }

    void Log::use_default_tag_settings()
    {
        s_enabled_tags = s_default_tag_settings;
    }

    void Log::de_init()
    {
        s_logger.reset();
        s_logger = nullptr;
    }

}