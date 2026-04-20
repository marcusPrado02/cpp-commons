#include "json_logger.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <format>

namespace cpp_commons::observability {

JsonLogger::JsonLogger(std::string service_name,
                       std::shared_ptr<spdlog::logger> logger)
    : service_name_(std::move(service_name))
    , logger_(logger ? std::move(logger)
                     : spdlog::stdout_color_mt(service_name_)) {
    logger_->set_pattern(R"({"ts":"%Y-%m-%dT%H:%M:%S.%e","level":"%l","svc":"%n","msg":"%v"})");
}

void JsonLogger::log(spdlog::level::level_enum lvl, std::string_view msg) const {
    const auto& ctx = current_correlation();
    if (!ctx.empty()) {
        logger_->log(lvl,
            std::format(R"({{"msg":"{}","cid":"{}","tid":"{}","rid":"{}"}})",
                msg,
                ctx.correlation_id,
                ctx.tenant_id,
                ctx.request_id));
    } else {
        logger_->log(lvl, msg);
    }
}

void JsonLogger::trace(std::string_view msg) const { log(spdlog::level::trace, msg); }
void JsonLogger::debug(std::string_view msg) const { log(spdlog::level::debug, msg); }
void JsonLogger::info(std::string_view msg)  const { log(spdlog::level::info,  msg); }
void JsonLogger::warn(std::string_view msg)  const { log(spdlog::level::warn,  msg); }
void JsonLogger::error(std::string_view msg) const { log(spdlog::level::err,   msg); }

} // namespace cpp_commons::observability
