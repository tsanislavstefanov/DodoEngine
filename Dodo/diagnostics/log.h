#pragma once

#include <spdlog/spdlog.h>

namespace Dodo {

    class Log
    {
    public:
        enum class Level
        {
            trace     ,
            info      ,
            warning   ,
            error     ,
            fatal     ,
            auto_count,
            none
        };

        struct TagDetails
        {
            bool Enabled = true;
            Level LevelFilter = Level::trace;
        };

        ////////////////////////////////////////////////////////////
        // TAG DETAILS MAP /////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        using TagDetailsMap = std::map<std::string_view, TagDetails>;

        static void init();
        static void use_default_tag_settings();
        static void de_init();
        static inline bool has_tag(const std::string_view tag) { return s_enabled_tags.find(tag) != s_enabled_tags.end(); }
        static inline TagDetailsMap& get_enabled_tags() { return s_enabled_tags; }
        static inline std::shared_ptr<spdlog::logger> get_logger() { return s_logger; }

        template<class... Args>
        static void print_message(Level level, std::format_string<Args...> format, Args&&... args);

        template<class... Args>
        static void print_message_tag(Level level, std::string_view tag, std::format_string<Args...> format, Args&&... args);

    private:
        static inline TagDetailsMap s_enabled_tags{};
        static inline std::shared_ptr<spdlog::logger> s_logger = nullptr;
    };

    template<class ...Args>
    inline void Log::print_message(Level level, std::format_string<Args...> format, Args&&... args)
    {
        const std::string message = std::format(format, std::forward<Args>(args)...);
        switch (level)
        {
            case Level::trace   : { s_logger->trace   ("{0}", message); break; }
            case Level::info    : { s_logger->info    ("{0}", message); break; }
            case Level::warning : { s_logger->warn    ("{0}", message); break; }
            case Level::error   : { s_logger->error   ("{0}", message); break; }
            case Level::fatal   : { s_logger->critical("{0}", message); break; }
            default             : { break; }
        }
    }

    template <class... Args>
    inline void Log::print_message_tag(Level level, std::string_view tag, std::format_string<Args...> format, Args&&... args)
    {
        if (!has_tag(tag))
        {
            print_message(Level::warning, "Tag [{0}] is not enabled or doesn't exist!", tag);
            return;
        }

        const TagDetails& tag_details = s_enabled_tags.at(tag);
        if (tag_details.Enabled && (tag_details.LevelFilter <= level))
        {
            const std::string message = std::format(format, std::forward<Args>(args)...);
            print_message(level, "[{0}] {1}", tag, message);
        }
    }

}

////////////////////////////////////////////////////////////////////
// TAG LOGS ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

#ifdef DODO_DEBUG
#   define DODO_LOG_TRACE_TAG(TAG, ...)   ::Dodo::Log::print_message_tag(::Dodo::Log::Level::trace  , TAG, __VA_ARGS__)
#   define DODO_LOG_INFO_TAG(TAG, ...)    ::Dodo::Log::print_message_tag(::Dodo::Log::Level::info   , TAG, __VA_ARGS__)
#   define DODO_LOG_WARNING_TAG(TAG, ...) ::Dodo::Log::print_message_tag(::Dodo::Log::Level::warning, TAG, __VA_ARGS__)
#   define DODO_LOG_ERROR_TAG(TAG, ...)   ::Dodo::Log::print_message_tag(::Dodo::Log::Level::error  , TAG, __VA_ARGS__)
#   define DODO_LOG_FATAL_TAG(TAG, ...)   ::Dodo::Log::print_message_tag(::Dodo::Log::Level::fatal  , TAG, __VA_ARGS__)
#else
#   define DODO_LOG_TRACE_TAG(...)
#   define DODO_LOG_INFO_TAG(...)
#   define DODO_LOG_WARNING_TAG(...)
#   define DODO_LOG_ERROR_TAG(...)
#   define DODO_LOG_FATAL_TAG(...)
#endif

////////////////////////////////////////////////////////////////////
// MESSAGE LOGS ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

#ifdef DODO_DEBUG
#   define DODO_LOG_TRACE(...)   ::Dodo::Log::print_message(::Dodo::Log::Level::trace  , __VA_ARGS__)
#   define DODO_LOG_INFO(...)    ::Dodo::Log::print_message(::Dodo::Log::Level::info   , __VA_ARGS__)
#   define DODO_LOG_WARNING(...) ::Dodo::Log::print_message(::Dodo::Log::Level::warning, __VA_ARGS__)
#   define DODO_LOG_ERROR(...)   ::Dodo::Log::print_message(::Dodo::Log::Level::error  , __VA_ARGS__)
#   define DODO_LOG_FATAL(...)   ::Dodo::Log::print_message(::Dodo::Log::Level::fatal  , __VA_ARGS__)
#else
#   define DODO_LOG_TRACE(...)
#   define DODO_LOG_INFO(...)
#   define DODO_LOG_WARNING(...)
#   define DODO_LOG_ERROR(...)
#   define DODO_LOG_FATAL(...)
#endif