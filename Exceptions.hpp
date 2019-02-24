#ifndef _COWR_EXCEPTIONS

#define _COWR_EXCEPTIONS

#include <exception>

class CoroutineError : public std::exception {
public:
    virtual const char* what() const noexcept
    {
        return "coroutine_base_error";
    }
};

class AwaitInMainCoroutineError : public CoroutineError {
public:
    virtual const char* what() const noexcept
    {
        return "await_in_main_coroutine_error";
    }
};

#endif
