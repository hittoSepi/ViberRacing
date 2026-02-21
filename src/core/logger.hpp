#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>

namespace viber {

class Logger {
public:
    static void init(const std::string& logFile = "logs/viber.log") {
        try {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(spdlog::level::trace);
            console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
            
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile, true);
            file_sink->set_level(spdlog::level::trace);
            file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v");
            
            std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
            
            auto logger = std::make_shared<spdlog::logger>("viber", sinks.begin(), sinks.end());
            logger->set_level(spdlog::level::debug);
            logger->flush_on(spdlog::level::info);
            
            spdlog::register_logger(logger);
            spdlog::set_default_logger(logger);
            
        } catch (const spdlog::spdlog_ex& ex) {
            fprintf(stderr, "Log initialization failed: %s\n", ex.what());
        }
    }
    
    static void setLevel(spdlog::level::level_enum level) {
        spdlog::set_level(level);
    }
    
    static void flush() {
        spdlog::default_logger()->flush();
    }
};

}
