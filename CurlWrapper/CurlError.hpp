#ifndef _COWR_CURLWRAPPER_CURLERROR

#define _COWR_CURLWRAPPER_CURLERROR

#include <exception>

class CurlError : public std::exception {
public:
    virtual const char* what() const noexcept
    {
        return "curl_base_error";
    }
};

class CurlGlobalInitError : public CurlError {
public:
    virtual const char* what() const noexcept
    {
        return "curl_global_init_error";
    }
};

class CurlEasyInitError : public CurlError {
public:
    virtual const char* what() const noexcept
    {
        return "curl_easy_init_error";
    }
};

class CurlMultiInitError : public CurlError {
public:
    virtual const char* what() const noexcept
    {
        return "curl_multi_init_error";
    }
};

class CurlMultiWaitError : public CurlError {
public:
    virtual const char* what() const noexcept
    {
        return "curl_multi_wait_error";
    }
};
#endif
