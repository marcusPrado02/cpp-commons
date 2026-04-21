/// @file matchers.hpp
/// @brief GTest/GMock matchers for `Result<T,E>` and `Option<T>`.
///
/// Usage: `EXPECT_THAT(result, IsOk())`, `EXPECT_THAT(opt, IsSomeWith(42))`.
#pragma once
#include <string>

#include <cpp_commons/kernel/option.hpp>
#include <cpp_commons/kernel/result.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace cpp_commons::testing {

// ── Result matchers ───────────────────────────────────────────────────────────

MATCHER(IsOk, "is a successful Result") {
    if (!arg.is_ok()) {
        *result_listener << "result is an error";
        return false;
    }
    return true;
}

MATCHER(IsErr, "is an error Result") {
    if (!arg.is_err()) {
        *result_listener << "result is ok";
        return false;
    }
    return true;
}

MATCHER_P(IsOkWith, expected, "is a successful Result with the expected value") {
    if (!arg.is_ok()) {
        *result_listener << "result is an error";
        return false;
    }
    if (arg.value() != expected) {
        *result_listener << "value is " << ::testing::PrintToString(arg.value());
        return false;
    }
    return true;
}

MATCHER_P(IsErrWith, expected, "is an error Result with the expected error") {
    if (!arg.is_err()) {
        *result_listener << "result is ok";
        return false;
    }
    if (arg.error() != expected) {
        *result_listener << "error is " << ::testing::PrintToString(arg.error());
        return false;
    }
    return true;
}

// ── Option matchers ───────────────────────────────────────────────────────────

MATCHER(IsSome, "is a non-empty Option") {
    if (!arg.has_value()) {
        *result_listener << "option is none";
        return false;
    }
    return true;
}

MATCHER(IsNone, "is an empty Option") {
    if (arg.has_value()) {
        *result_listener << "option has a value";
        return false;
    }
    return true;
}

MATCHER_P(IsSomeWith, expected, "is a non-empty Option with the expected value") {
    if (!arg.has_value()) {
        *result_listener << "option is none";
        return false;
    }
    if (arg.value() != expected) {
        *result_listener << "value is " << ::testing::PrintToString(arg.value());
        return false;
    }
    return true;
}

}  // namespace cpp_commons::testing
