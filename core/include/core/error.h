#pragma once

#include <format>
#include <string>

namespace core {

class Error {
  public:
    explicit Error(std::string msg) : m_message(std::move(msg)) {}

    template<typename... Args>
    Error(std::format_string<Args...> fmt_string, Args&&... args) :
        m_message(std::format(fmt_string, std::forward<Args>(args)...)) {}

    [[nodiscard]] std::string_view message() const noexcept {
        return m_message;
    }

  private:
    std::string m_message;
};

}  // namespace core
