#pragma once
#include <string_view>

namespace w {

template<typename Message>
struct [[nodiscard]] basic_result {
public:
    Message message{};
};

template<typename Message>
static constexpr basic_result<Message> success{};

struct error_t {
};
constexpr error_t error{};

template<typename RetTy>
struct [[nodiscard]] result {
    basic_result<std::string_view> error{};
    RetTy value{};

    constexpr result() = default;
    constexpr result(RetTy&& ret) noexcept
        : value(std::move(ret)) { }
    constexpr result(std::string_view error, error_t) noexcept
        : error{ error } { }

    constexpr operator bool() const noexcept
    {
        return !error.message.empty();
    }
};
} // namespace w