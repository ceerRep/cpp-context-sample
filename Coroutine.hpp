#ifndef _COWR_COROUTINE

#define _COWR_COROUTINE

#pragma GCC push_options
#pragma GCC optimize("O0")

#include <functional>
#include <type_traits>
#include <vector>

#include "Context.hpp"
#include "Exceptions.hpp"

inline std::vector<unsigned char> context_data(sizeof(nullptr));

template <typename Callable>
std::invoke_result_t<Callable> cowr_await(const Callable& func)
{

    if (now_context_id == 1)
        throw AwaitInMainCoroutineError();

    using true_ret_type = std::invoke_result_t<Callable>;
    constexpr bool is_ref = std::is_reference_v<true_ret_type>;
    using ret_type = std::conditional_t<is_ref,
        std::add_pointer_t<std::remove_reference_t<true_ret_type>>,
        true_ret_type>;

    constexpr bool is_void = std::is_void_v<ret_type>;

    if constexpr (!is_void) {
        if (sizeof(ret_type) > context_data.size()) {
            context_data.resize(sizeof(ret_type));
        }
    }

    Context::getNowContext()->Call([&]() -> void {
        if constexpr (is_void)
            func();
        else {
            ret_type ret;
            if constexpr (is_ref)
                ret = &(func());
            else
                ret = func();
            new ((ret_type*)(context_data.data())) ret_type(ret);

            // if constexpr (is_ref)
            //     new ((ret_type*)(context_data.data())) ret_type(&(func()));
            // else
            //     new ((ret_type*)(context_data.data())) ret_type(func());
        }
    });

    if (exception_occured) {
        exception_occured = false;
        std::rethrow_exception(now_exception);
    }

    if (is_yield) {
        if constexpr (is_void)
            ;
        else if constexpr (is_ref)
            return **(ret_type*)(context_data.data());
        else
            return **(ret_type**)(context_data.data());
    }

    if constexpr (!is_void) {
        ret_type ret = *(ret_type*)(context_data.data());
        // std::clog << "ret:" << &ret << std::endl;

        // ret_type is pointer if true_ret_type is reference, so won't destroy
        std::destroy_at((ret_type*)(context_data.data()));

        if constexpr (is_ref)
            return *ret;
        else
            return ret;
    } else
        return;
}

void cowr_yield()
{
    Context::getNowContext()->Yield();
}

template <typename ret_type>
void cowr_yield(ret_type&& ret)
{
    constexpr bool is_ref = std::is_reference_v<ret_type>;

    if constexpr (is_ref) {
        *(std::remove_reference_t<ret_type>**)(context_data.data()) = &ret;
    } else {
        *(ret_type**)(context_data.data()) = &ret;
    }

    Context::getNowContext()->Yield();
}

template <typename Callable>
std::invoke_result_t<Callable> cowr_resume(const Callable& func)
{
    using true_ret_type = std::invoke_result_t<Callable>;
    constexpr bool is_ref = std::is_reference_v<true_ret_type>;
    using ret_type = std::conditional_t<is_ref,
        std::add_pointer_t<std::remove_reference_t<true_ret_type>>,
        true_ret_type>;
    constexpr bool is_void = std::is_void_v<ret_type>;

    Context::getNowContext()->Resume();

    if (is_yield) {
        if constexpr (is_void)
            ;
        else if constexpr (is_ref)
            return **(ret_type*)(context_data.data());
        else
            return **(ret_type**)(context_data.data());
    }
    if constexpr (!is_void) {
        ret_type ret = *(ret_type*)(context_data.data());

        //same as cowr_await
        std::destroy_at((ret_type*)(context_data.data()));

        if constexpr (is_ref)
            return *ret;
        else
            return ret;
    }
}

#if 0

#define AWAIT(expr) ({                                                                                 \
    if (sizeof(decltype(expr)) > context_data.size()) {                                                \
        context_data.resize(sizeof(decltype(expr)));                                                   \
    }                                                                                                  \
    Context::getNowContext()->Call([&]() -> void { *(decltype(expr)*)(context_data.data()) = expr; }); \
    *(decltype(expr)*)(context_data.data());                                                           \
})

#define YIELD(expr) ({                                  \
    if (sizeof(decltype(expr)) > context_data.size()) { \
        context_data.resize(sizeof(decltype(expr)));    \
    }                                                   \
    *(decltype(expr)*)(context_data.data()) = expr;     \
    Context::getNowContext()->Yield();                  \
})

#define RESUME(expr) ({                      \
    Context::getNowContext()->Resume();      \
    *(decltype(expr)*)(context_data.data()); \
})

#endif

#pragma GCC pop_options

#endif
