#pragma once

#include <expected>
#include <functional>
#include <print>

namespace core {

struct Unit {};

template<typename T, typename E>
class Result;

namespace detail {
    template<typename T>
    struct is_result: std::false_type {};

    // Specialization - true for Result types
    template<typename T, typename E>
    struct is_result<Result<T, E>>: std::true_type {};

    // Convenient variable template
    template<typename T>
    inline constexpr bool is_result_v = is_result<T>::value;

    template<typename T>
    concept result_type = is_result_v<T>;
}  // namespace detail

/**
 * @brief A Result type that holds either a success value (T) or an error value
 * (E)
 *
 * Result provides a type-safe way to handle operations that may fail without
 * exceptions. It forces explicit error handling and enables functional
 * composition of fallible operations.
 *
 * @tparam T The success value type
 * @tparam E The error value type
 */
template<typename T, typename E>
class [[nodiscard]] Result {
  public:
    /**
   * @brief Construct a Result with a success value
   * @param value The success value to store
   */
    Result(const T& value) : Result(ok_tag {}, value) {}

    Result(T&& value) : Result(ok_tag {}, std::move(value)) {}

    /**
   * @brief Construct a Result with an error value
   * @param error The error value to store
   * @note Only enabled when T and E are different types
   */
    Result(const E& error)
        requires(!std::is_same_v<T, E>)
        : Result(err_tag {}, error) {}

    Result(E&& error)
        requires(!std::is_same_v<T, E>)
        : Result(err_tag {}, std::move(error)) {}

    /**
   * @brief Check if the Result contains a success value
   * @return true if holding a success value, false if holding an error
   */
    bool is_ok() const noexcept {
        return m_data.has_value();
    }

    /**
   * @brief Check if the Result contains an error value
   * @return true if holding an error, false if holding a success value
   */
    bool is_err() const noexcept {
        return !m_data.has_value();
    }

    /**
   * @brief Convert to bool, returns true if Result is Ok
   * @return true if holding a success value
   * @code
   * if (auto result = divide(10, 2)) {
   *     // result is Ok
   * }
   * @endcode
   */
    explicit operator bool() const noexcept {
        return m_data.has_value();
    }

    /**
   * @brief Extract the success value, aborts if Result is Err
   * @return Reference to the contained success value
   * @note Program will abort with error message if the Result contains an error
   * @code
   * auto value = divide(10, 2).unwrap(); // Ok: returns 5
   * auto bad = divide(10, 0).unwrap();   // Aborts program!
   * @endcode
   */
    T& unwrap() & {
        if (!m_data.has_value()) {
            std::println("Called unwrap() on an error Result");
            std::abort();
        }
        return m_data.value();
    }

    const T& unwrap() const& {
        if (!m_data.has_value()) {
            std::println("Called unwrap() on an error Result");
            std::abort();
        }
        return m_data.value();
    }

    T&& unwrap() && {
        if (!m_data.has_value()) {
            std::println("Called unwrap() on an error Result");
            std::abort();
        }
        return std::move(m_data.value());
    }

    /**
   * @brief Extract the error value, aborts if Result is Ok
   * @return Reference to the contained error value
   * @note Program will abort with error message if the Result contains a
   * success value
   */
    E& unwrap_err() & {
        if (m_data.has_value()) {
            std::println("Called unwrap_err() on an ok Result");
            std::abort();
        }
        return m_data.error();
    }

    const E& unwrap_err() const& {
        if (m_data.has_value()) {
            std::println("Called unwrap_err() on an ok Result");
            std::abort();
        }
        return m_data.error();
    }

    E&& unwrap_err() && {
        if (m_data.has_value()) {
            std::println("Called unwrap_err() on an ok Result");
            std::abort();
        }
        return std::move(m_data.error());
    }

    /**
   * @brief Access the success value (same as unwrap but matches std::expected)
   * @return Reference to the contained success value
   * @throw std::bad_expected_access if Result contains an error
   */
    T& value() & {
        return m_data.value();
    }

    const T& value() const& {
        return m_data.value();
    }

    T&& value() && {
        return std::move(m_data.value());
    }

    /**
   * @brief Access the error value
   * @return Reference to the contained error value
   * @note Behavior is undefined if Result contains a success value
   */
    E& error() & {
        return m_data.error();
    }

    const E& error() const& {
        return m_data.error();
    }

    E&& error() && {
        return std::move(m_data.error());
    }

    /**
   * @brief Get the success value or a default if Result is Err
   * @param default_value Value to return if Result contains an error
   * @return The success value or default_value
   * @code
   * auto value = parse_int("42").unwrap_or(-1);  // returns 42
   * auto value = parse_int("abc").unwrap_or(-1); // returns -1
   * @endcode
   */
    T unwrap_or(T&& default_value) const& {
        return m_data.value_or(std::forward<T>(default_value));
    }

    T unwrap_or(T&& default_value) && {
        return std::move(m_data).value_or(std::forward<T>(default_value));
    }

    /**
   * @brief Get the success value or compute a default from the error
   * @param f Function that takes the error and returns a T
   * @return The success value or the result of f(error)
   * @code
   * auto value = divide(10, 0).unwrap_or_else([](const auto& err) {
   *     log_error(err);
   *     return 0;
   * });
   * @endcode
   */
    template<typename F>
    T unwrap_or_else(F&& f) const& {
        if (m_data.has_value()) {
            return m_data.value();
        }
        return std::invoke(std::forward<F>(f), m_data.error());
    }

    template<typename F>
    T unwrap_or_else(F&& f) && {
        if (m_data.has_value()) {
            return std::move(m_data.value());
        }
        return std::invoke(std::forward<F>(f), std::move(m_data.error()));
    }

    /**
   * @brief Chain operations that return Results
   * @param f Function that takes T and returns Result<U, E> (or just U)
   * @return Result<U, E> from f if Ok, or propagated error if Err
   * @code
   * auto result = parse_int("42")
   *     .and_then([](int x) { return divide(100, x); })
   *     .and_then([](int x) { return Result<int, string>(x * 2); });
   * @endcode
   */
    template<typename F>
    auto and_then(F&& f) const& -> decltype(auto) {
        using invoke_result = std::invoke_result_t<F, const T&>;

        if (m_data.has_value()) {
            if constexpr (detail::is_result_v<invoke_result>) {
                return std::invoke(std::forward<F>(f), m_data.value());
            } else {
                using return_type = Result<invoke_result, E>;
                return return_type(
                    std::invoke(std::forward<F>(f), m_data.value())
                );
            }
        }

        if constexpr (detail::is_result_v<invoke_result>) {
            return invoke_result(m_data.error());
        } else {
            using return_type = Result<invoke_result, E>;
            return return_type(m_data.error());
        }
    }

    template<typename F>
    auto and_then(F&& f) && -> decltype(auto) {
        using invoke_result = std::invoke_result_t<F, T&&>;

        if (m_data.has_value()) {
            if constexpr (detail::is_result_v<invoke_result>) {
                return std::invoke(
                    std::forward<F>(f),
                    std::move(m_data.value())
                );
            } else {
                using return_type = Result<invoke_result, E>;
                return return_type(
                    std::invoke(std::forward<F>(f), std::move(m_data.value()))
                );
            }
        }

        if constexpr (detail::is_result_v<invoke_result>) {
            return invoke_result(std::move(m_data.error()));
        } else {
            using return_type = Result<invoke_result, E>;
            return return_type(std::move(m_data.error()));
        }
    }

    /**
   * @brief Handle errors by potentially recovering with a new Result
   * @param f Function that takes E and returns Result<T, U> (or just U)
   * @return Original Result if Ok, or the result of f(error) if Err
   * @code
   * auto result = read_file("config.txt")
   *     .or_else([](auto& err) { return read_file("default.txt"); });
   * @endcode
   */
    template<typename F>
    auto or_else(F&& f) const& -> decltype(auto) {
        using invoke_result = std::invoke_result_t<F, const E&>;

        if (!m_data.has_value()) {
            if constexpr (detail::is_result_v<invoke_result>) {
                return std::invoke(std::forward<F>(f), m_data.error());
            } else {
                using return_type = Result<T, invoke_result>;
                return return_type(
                    std::invoke(std::forward<F>(f), m_data.error())
                );
            }
        }

        if constexpr (detail::is_result_v<invoke_result>) {
            return invoke_result(m_data.value());
        } else {
            using return_type = Result<T, invoke_result>;
            return return_type(m_data.value());
        }
    }

    /**
   * @brief Transform the success value inside the Result
   * @param f Function that transforms T to U
   * @return Result<U, E> with transformed value if Ok, or propagated error
   * @code
   * auto length = parse_int("42")
   *     .map([](int x) { return x * 2; })    // Result<int, Error>
   *     .map([](int x) { return std::to_string(x); }); // Result<string, Error>
   * @endcode
   */
    template<typename F>
    auto map(F&& f) const& -> Result<std::invoke_result_t<F, const T&>, E> {
        using U = std::invoke_result_t<F, const T&>;
        if (m_data.has_value()) {
            return Result<U, E>(
                std::invoke(std::forward<F>(f), m_data.value())
            );
        }
        return Result<U, E>(m_data.error());
    }

    template<typename F>
    auto map(F&& f) && -> Result<std::invoke_result_t<F, T&&>, E> {
        using U = std::invoke_result_t<F, T&&>;
        if (m_data.has_value()) {
            return Result<U, E>(
                std::invoke(std::forward<F>(f), std::move(m_data.value()))
            );
        }
        return Result<U, E>(std::move(m_data.error()));
    }

    /**
   * @brief Transform the error value inside the Result
   * @param f Function that transforms E to U
   * @return Result<T, U> with same value if Ok, or transformed error if Err
   * @code
   * auto result = parse_int("abc")
   *     .map_err([](ParseError& e) { return format_error(e); });
   * @endcode
   */
    template<typename F>
    auto map_err(F&& f) const& -> Result<T, std::invoke_result_t<F, const E&>> {
        using U = std::invoke_result_t<F, const E&>;
        if (!m_data.has_value()) {
            return Result<T, U>(
                std::invoke(std::forward<F>(f), m_data.error())
            );
        }
        return Result<T, U>(m_data.value());
    }

    template<typename F>
    auto map_err(F&& f) && -> Result<T, std::invoke_result_t<F, E&&>> {
        using U = std::invoke_result_t<F, E&&>;
        if (!m_data.has_value()) {
            return Result<T, U>(
                std::invoke(std::forward<F>(f), std::move(m_data.error()))
            );
        }
        return Result<T, U>(std::move(m_data.value()));
    }

    /**
   * @brief Pattern match on Result with separate handlers for Ok and Err
   * @param on_success Function to handle success case
   * @param on_error Function to handle error case
   * @return Common type of both function results
   * @code
   * int result = divide(10, 2).match(
   *     [](int value) { return value; },
   *     [](const Error& err) {
   *         std::println("Error: {}", err);
   *         return -1;
   *     }
   * );
   * @endcode
   */
    template<typename F, typename G>
    auto match(F&& on_success, G&& on_error) const& -> std::common_type_t<
        std::invoke_result_t<F, const T&>,
        std::invoke_result_t<G, const E&>> {
        if (m_data.has_value()) {
            return std::forward<F>(on_success)(m_data.value());
        } else {
            return std::forward<G>(on_error)(m_data.error());
        }
    }

    template<typename F, typename G>
    auto match(F&& on_success, G&& on_error) && -> std::common_type_t<
        std::invoke_result_t<F, T&&>,
        std::invoke_result_t<G, E&&>> {
        if (m_data.has_value()) {
            return std::forward<F>(on_success)(std::move(m_data.value()));
        } else {
            return std::forward<G>(on_error)(std::move(m_data.error()));
        }
    }

  private:
    std::expected<T, E> m_data;

    struct ok_tag {};

    struct err_tag {};

    Result(ok_tag, const T& value) : m_data(value) {}

    Result(ok_tag, T&& value) : m_data(std::move(value)) {}

    Result(err_tag, const E& error) : m_data(std::unexpect, error) {}

    Result(err_tag, E&& error) : m_data(std::unexpect, std::move(error)) {}
};

#ifdef _MSC_VER
    #define TRY(expr) \
        [&]() -> auto { \
            auto&& __try_result = (expr); \
            if (!__try_result.is_ok()) { \
                return std::move(__try_result).error(); \
            } \
            return std::move(__try_result).unwrap(); \
        }()
#else
    #define TRY(expr) \
        ({ \
            auto&& __try_result = (expr); \
            if (!__try_result.is_ok()) { \
                return std::move(__try_result).error(); \
            } \
            std::move(__try_result).unwrap(); \
        })
#endif

#define TRY_ASSIGN_IMPL(var, expr, assignment) \
    auto __try_result_##var = (expr); \
    if (!__try_result_##var.is_ok()) \
        return std::move(__try_result_##var).unwrap_err(); \
    assignment

#define TRY_ASSIGN_TYPE(type, var, expr) \
    TRY_ASSIGN_IMPL( \
        var, expr, type var = std::move(__try_result_##var).unwrap(); \
    )

#define TRY_ASSIGN_MOVE(var, expr) \
    TRY_ASSIGN_IMPL( \
        var, expr, auto var = std::move(__try_result_##var).unwrap(); \
    )

#define TRY_ASSIGN_INTO(var, expr) \
    do { \
        auto __try_result = (expr); \
        if (!__try_result.is_ok()) \
            return std::move(__try_result).unwrap_err(); \
        var = std::move(__try_result).unwrap(); \
    } while (0)

#define TRY_EXEC(expr) \
    do { \
        auto __try_result = (expr); \
        if (!__try_result.is_ok()) \
            return std::move(__try_result).unwrap_err(); \
    } while (0)

}  // namespace core
