#pragma once

#include <tuple>

namespace toolkit::detail {

template<typename T>
struct function_traits;

template<typename R, typename... Args>
struct function_traits<R(Args...)> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
};

template<typename R, typename... Args>
struct function_traits<R (*)(Args...)> : function_traits<R(Args...)> {};

template<typename R, typename... Args>
struct function_traits<R (&)(Args...)> : function_traits<R(Args...)> {};

template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...)> : function_traits<R(Args...)> {};

template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) const> : function_traits<R(Args...)> {};

template<typename T>
struct function_traits : function_traits<decltype(&T::operator())> {};

template<typename F>
using first_arg_t = std::remove_const_t<std::remove_reference_t<
    std::tuple_element_t<0, typename function_traits<std::decay_t<F>>::args_tuple>>>;

}  // namespace toolkit::detail
