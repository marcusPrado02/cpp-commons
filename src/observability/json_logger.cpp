#include "json_logger.hpp"

#include <format>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace cpp_commons::observability {

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
JsonLogger::JsonLogger(std::string service_name, std::shared_ptr<spdlog::logger> logger)
    : service_name_(std::move(service_name)),
      logger_(logger ? std::move(logger) : spdlog::stdout_color_mt(service_name_)) {
    logger_->set_pattern(R"({"ts":"%Y-%m-%dT%H:%M:%S.%e","level":"%l","svc":"%n","msg":"%v"})");
}

void JsonLogger::log(spdlog::level::level_enum lvl, std::string_view msg) const {
    const auto& ctx = current_correlation();
    if (!ctx.empty()) {
        logger_->log(lvl, std::format(R"({{"msg":"{}","cid":"{}","tid":"{}","rid":"{}"}})", msg,
                                      ctx.correlation_id, ctx.tenant_id, ctx.request_id));
        return;
    }
    logger_->log(lvl, msg);
}

void JsonLogger::trace(std::string_view msg) const {
    log(spdlog::level::trace, msg);
}
void JsonLogger::debug(std::string_view msg) const {
    log(spdlog::level::debug, msg);
}
void JsonLogger::info(std::string_view msg) const {
    log(spdlog::level::info, msg);
}
void JsonLogger::warn(std::string_view msg) const {
    log(spdlog::level::warn, msg);
}
void JsonLogger::error(std::string_view msg) const {
    log(spdlog::level::err, msg);
}

void JsonLogger::trace(std::string_view msg, Fields f) const {
    log(spdlog::level::trace, msg, f);
}
void JsonLogger::debug(std::string_view msg, Fields f) const {
    log(spdlog::level::debug, msg, f);
}
void JsonLogger::info(std::string_view msg, Fields f) const {
    log(spdlog::level::info, msg, f);
}
void JsonLogger::warn(std::string_view msg, Fields f) const {
    log(spdlog::level::warn, msg, f);
}
void JsonLogger::error(std::string_view msg, Fields f) const {
    log(spdlog::level::err, msg, f);
}

void JsonLogger::log(spdlog::level::level_enum lvl, std::string_view msg, Fields fields) const {
    const auto& ctx = current_correlation();
    std::string entry = std::format(R"({{"msg":"{}")", msg);
    if (!ctx.empty()) {
        entry += std::format(R"(,"cid":"{}","tid":"{}","rid":"{}")", ctx.correlation_id,
                             ctx.tenant_id, ctx.request_id);
    }
    for (const auto& [k, v] : fields) {
        entry += std::format(R"(,"{}":"{}")", k, v);
    }
    entry += '}';
    logger_->log(lvl, entry);
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
JsonLogger JsonLogger::with_file(std::string service_name, const std::string& file_path,
                                 std::size_t max_size_mb, std::size_t max_files) {
    // NOLINTEND(bugprone-easily-swappable-parameters)
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    static constexpr std::size_t kBytesPerMb = 1024UZ * 1024UZ;
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        file_path, max_size_mb * kBytesPerMb, max_files);
    auto logger = std::make_shared<spdlog::logger>(service_name,
                                                   spdlog::sinks_init_list{stdout_sink, file_sink});
    logger->set_pattern(R"({"ts":"%Y-%m-%dT%H:%M:%S.%e","level":"%l","svc":"%n","msg":"%v"})");
    return JsonLogger{service_name, logger};
}

}  // namespace cpp_commons::observability
