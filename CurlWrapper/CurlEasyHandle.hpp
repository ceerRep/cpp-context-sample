#ifndef _COWR_CURLWRAPPER_CURLEASYHANDLE

#define _COWR_CURLWRAPPER_CURLEASYHANDLE

#include <functional>
#include <iostream>
#include <string>

#include <curl/curl.h>

#include "CurlError.hpp"

class CurlEasyHandle {
private:
    CURL* handle;
    std::function<size_t(void*, size_t, size_t, CurlEasyHandle&)> on_data_receive = curlDefaultDataReceiveHandler;
    std::function<void(CurlEasyHandle&)> on_request_done = curlDefaultRequestDoneHandler;

public:
    enum {
        ON_DATA_RECEIVE,
        ON_REQUEST_DONE
    };

    CurlEasyHandle()
        : handle(curl_easy_init())
    {
        if (!handle)
            throw(CurlEasyInitError());

        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curlHandleFunc);
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);

        setCanRedirect(true);
    }

    ~CurlEasyHandle()
    {
        curl_easy_cleanup(handle);
    }

    void onRequestDone()
    {
        on_request_done(*this);
    }

    template <typename T>
    CURLcode setOption(CURLoption option, T args)
    {
        return curl_easy_setopt(handle, option, args);
    }

    CURLcode setURL(std::string URI)
    {
        return curl_easy_setopt(handle, CURLOPT_URL, URI.c_str());
    }

    CURLcode setDataReceiveHandler(std::function<size_t(void*, size_t, size_t, CurlEasyHandle&)> handler)
    {
        on_data_receive = handler;
        return CURLE_OK;
    }

    CURLcode setRequestDoneHandler(std::function<void(CurlEasyHandle&)> handler)
    {
        on_request_done = handler;
        return CURLE_OK;
    }

    CURLcode setCanRedirect(bool redirect)
    {
        return curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, long(redirect));
    }

    CURLcode perform()
    {
        return curl_easy_perform(handle);
    }

    operator CURL*()
    {
        return handle;
    }

    static size_t curlDefaultDataReceiveHandler(void* buffer, size_t size, size_t nmemb, CurlEasyHandle& easy_handle)
    {
        std::cout.write((const char*)buffer, nmemb);
        std::cout.flush();
        return nmemb;
    }

    static void curlDefaultRequestDoneHandler(CurlEasyHandle& easy_handle)
    {
    }

    static size_t curlHandleFunc(void* buffer, size_t size, size_t nmemb, void* userp)
    {
        return ((CurlEasyHandle*)userp)->on_data_receive(buffer, size, nmemb, *(CurlEasyHandle*)userp);
    }
};

#endif
