#ifndef _COWR_CURLWRAPPER_CURLPROXY

#define _COWR_CURLWRAPPER_CURLPROXY

#include <string>

struct CurlProxy {
    std::string proxy;

    CurlProxy(std::string proxy = "", long port = 0)
        : proxy(proxy)
    {
    }

    CurlProxy& setProxy(std::string server)
    {
        proxy = server;
        return *this;
    }
};

struct CurlHTTPProxy : public CurlProxy {
    CurlHTTPProxy(std::string proxy = "")
        : CurlProxy(proxy)
    {
    }
};

#endif
