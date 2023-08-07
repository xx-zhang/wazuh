#ifndef _H_LOGGING
#define _H_LOGGING

#include <iostream>
#include <map>

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace logging
{

constexpr char DEFAULT_LOG_PATH[] {"/dev/stderr"};
constexpr char DEFAULT_LOG_HEADER[] {"%Y-%m-%d %T.%e %P:%t %l: %v"};
constexpr char DEFAULT_LOG_LEVEL[] {"info"}; ///< "trace", "debug", "info", "warning", "error", "critical", "off"
constexpr uint32_t DEFAULT_LOG_THREADS {0};  ///< Quantity of dedicated threads, 0 means no dedicated threads
constexpr uint32_t DEFAULT_LOG_THREADS_QUEUE_SIZE {8192}; ///< Size in bytes
constexpr uint32_t DEFAULT_LOG_FLUSH_INTERVAL {1};        ///< Value in ms

/**
 * @brief Used to convert the name of the log level to its corresponding value.
 */
const std::map<std::string, spdlog::level::level_enum> SEVERITY_LEVEL {{"trace", spdlog::level::trace},
                                                                       {"debug", spdlog::level::debug},
                                                                       {"info", spdlog::level::info},
                                                                       {"warning", spdlog::level::warn},
                                                                       {"error", spdlog::level::err},
                                                                       {"critical", spdlog::level::critical},
                                                                       {"off", spdlog::level::off}};

struct LoggingConfig
{
    std::string filePath {DEFAULT_LOG_PATH};
    // To know more about the format parameters, please see: https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
    std::string headerFormat {DEFAULT_LOG_HEADER};
    std::string logLevel {DEFAULT_LOG_LEVEL};
    const uint32_t flushInterval {DEFAULT_LOG_FLUSH_INTERVAL};     ///< Value in ms
    const uint32_t dedicatedThreads {DEFAULT_LOG_THREADS};         ///< 0 means no dedicated threads,
                                                                   ///< if one or more then logsQueueSize takes effect
    const uint32_t logsQueueSize {DEFAULT_LOG_THREADS_QUEUE_SIZE}; ///< Logs queue size to be processed by the dedicated
                                                                   ///< threads (has to be 1 or more)
};

// TODO: This emulates a global variable to fasten the access to the "default" logger, it can be improved
inline auto getDefaultLogger(void)
{
    static auto defaultLogger = spdlog::get("default").get();
    return defaultLogger;
}

static inline void loggingInit(LoggingConfig& cfg)
{
    const bool doTruncateFile {false};
    try
    {
        if (0 < cfg.dedicatedThreads)
        {
            // Here we set the amount of DEDICATED threads
            spdlog::init_thread_pool(cfg.logsQueueSize, cfg.dedicatedThreads);
        }
        spdlog::flush_every(std::chrono::milliseconds(cfg.flushInterval));

        // Logger initialization ("default" is the logger name, it can be any custom name)
        if (!cfg.filePath.empty())
        {
            spdlog::basic_logger_mt("default", cfg.filePath, doTruncateFile);
        }
        else
        {
            auto logger = spdlog::stdout_color_mt("default");
        }
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        throw std::runtime_error(fmt::format("Log initialization failed: {}", ex.what()));
    }

    spdlog::level::level_enum level {SEVERITY_LEVEL.at(DEFAULT_LOG_LEVEL)};
    try
    {
        level = SEVERITY_LEVEL.at(cfg.logLevel);
    }
    catch (const std::out_of_range& e)
    {
        std::cerr << "An error ocurred while setting the log level '" << cfg.logLevel << "': " << e.what()
                  << ". Default logging level '" << DEFAULT_LOG_LEVEL << "' will be used instead." << std::endl;
    }

    getDefaultLogger()->set_level(level);
    getDefaultLogger()->flush_on(spdlog::level::err);
    getDefaultLogger()->set_pattern(cfg.headerFormat);
}

inline void testInit()
{
    static bool initialized = false;

    if (!initialized)
    {
        // Logging setup
        LoggingConfig logConfig;
        logConfig.logLevel = "off";
        logConfig.filePath = "";
        loggingInit(logConfig);
        initialized = true;
    }
}

} // namespace logging

/**
 * @brief Used for logging at different levels.
 */
#define LOG_TRACE(msg, ...)    logging::getDefaultLogger()->trace(msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...)    logging::getDefaultLogger()->debug(msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...)     logging::getDefaultLogger()->info(msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...)  logging::getDefaultLogger()->warn(msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...)    logging::getDefaultLogger()->error(msg, ##__VA_ARGS__)
#define LOG_CRITICAL(msg, ...) logging::getDefaultLogger()->critical(msg, ##__VA_ARGS__)

#endif
