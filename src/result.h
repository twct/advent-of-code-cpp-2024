#ifndef RESULT_H
#define RESULT_H

#include <assert.h>
#include <spdlog/spdlog.h>

#include <stdexcept>
#include <type_traits>
#include <variant>

struct Unit {};

template<typename T, typename E>
class [[nodiscard]] Result {
  public:
    Result(const T& value) : Result(ok_tag {}, value) {}

    Result(T&& value) : Result(ok_tag {}, std::move(value)) {}

    template<typename U = E, typename = std::enable_if_t<!std::is_same_v<T, U>>>
    Result(const E& error) : Result(err_tag {}, error) {}

    template<typename U = E, typename = std::enable_if_t<!std::is_same_v<T, U>>>
    Result(E&& error) : Result(err_tag {}, std::move(error)) {}

    bool is_ok() const {
        return std::holds_alternative<T>(m_data);
    }

    bool is_err() const {
        return std::holds_alternative<E>(m_data);
    }

    const T& expect(const std::string& message) const& {
        if (is_ok()) {
            return unwrap();
        }
        spdlog::error("{}: {}", message, unwrap_err().message());
        std::abort();
    }

    T&& expect(const std::string& message) && {
        if (is_ok()) {
            return std::move(*this).unwrap();
        }
        spdlog::error("{}: {}", message, unwrap_err().message());
        std::abort();
    }

    const T& unwrap() const& {
        if (!is_ok()) {
            spdlog::error("Called unwrap on an Err value");
            std::abort();
        }
        return std::get<T>(m_data);
    }

    T&& unwrap() && {
        if (!is_ok()) {
            spdlog::error("Called unwrap on an Err value");
            std::abort();
        }
        return std::move(std::get<T>(m_data));
    }

    const T& unwrap_ok() const& {
        return unwrap();
    }

    T&& unwrap_ok() && {
        return std::move(*this).unwrap();
    }

    const E& unwrap_err() const& {
        assert(is_err() && "Called unwrap_err on an Ok value");
        return std::get<E>(m_data);
    }

    E&& unwrap_err() && {
        assert(is_err() && "Called unwrap_err on an Ok value");
        return std::move(std::get<E>(m_data));
    }

    template<typename F>
    auto map(F&& f) const -> Result<decltype(f(std::declval<T>())), E> {
        if (is_ok()) {
            return Result<decltype(f(std::declval<T>())), E>(f(unwrap()));
        }
        return Result<decltype(f(std::declval<T>())), E>(unwrap_err());
    }

    template<typename F>
    auto map_err(F&& f) const -> Result<T, decltype(f(std::declval<E>()))> {
        if (is_err()) {
            return Result<T, decltype(f(std::declval<E>()))>(f(unwrap_err()));
        }
        return Result<T, decltype(f(std::declval<E>()))>(unwrap());
    }

    template<typename F>
    auto and_then(F&& f) const {
        if (is_ok()) {
            if constexpr (is_result<decltype(f(std::declval<T>()))>::value) {
                return f(unwrap());
            } else {
                return Result<decltype(f(std::declval<T>())), E>(f(unwrap()));
            }
        }
        return Result<decltype(f(std::declval<T>())), E>(unwrap_err());
    }

    template<typename U>
    T unwrap_or(U&& default_value) const {
        if (is_ok()) {
            return unwrap();
        }
        return static_cast<T>(std::forward<U>(default_value));
    }

    template<typename F>
    T unwrap_or_else(F&& f) const {
        if (is_ok()) {
            return unwrap();
        }
        return f(unwrap_err());
    }

  private:
    std::variant<T, E> m_data;

    struct ok_tag {};

    struct err_tag {};

    template<typename U>
    struct is_result: std::false_type {};

    template<typename U, typename V>
    struct is_result<Result<U, V>>: std::true_type {};

    Result(ok_tag, const T& value) : m_data(value) {}

    Result(ok_tag, T&& value) : m_data(std::move(value)) {}

    Result(err_tag, const E& error) : m_data(error) {}

    Result(err_tag, E&& error) : m_data(std::move(error)) {}
};

template<typename T, typename E>
Result<T, E> Ok(T&& value) {
    return Result<T, E>(std::forward<T>(value));
}

template<typename T, typename E>
Result<T, E> Err(E&& error) {
    return Result<T, E>(std::forward<E>(error));
}

// MSVC compat :(
#define TRY_ASSIGN(var, expr) \
    auto var##_result = (expr); \
    if (!var##_result.is_ok()) \
        return std::move(var##_result).unwrap_err(); \
    auto var = var##_result.unwrap();

#define TRY_ASSIGN_TYPE(type, var, expr) \
    auto&& _result_##var = (expr); \
    if (!_result_##var.is_ok()) \
        return std::move(_result_##var).unwrap_err(); \
    type var = _result_##var.unwrap();

#define TRY_ASSIGN_MOVE(var, expr) \
    auto var##_result = (expr); \
    if (!var##_result.is_ok()) \
        return std::move(var##_result).unwrap_err(); \
    auto var = std::move(var##_result).unwrap();

#define TRY_ASSIGN_INTO(var, expr) \
    auto var##_result = (expr); \
    if (!var##_result.is_ok()) \
        return std::move(var##_result).unwrap_err(); \
    var = std::move(var##_result).unwrap();

#if defined(_MSC_VER)
    #define TRY(expr) \
        do { \
            auto&& _result = (expr); \
            if (!_result.is_ok()) \
                return std::move(_result).unwrap_err(); \
            _result.unwrap(); \
        } while (0)
#else
    #define TRY(expr) \
        ({ \
            auto _result = (expr); \
            if (!_result.is_ok()) \
                return _result.unwrap_err(); \
            _result.unwrap(); \
        })
#endif

#endif
