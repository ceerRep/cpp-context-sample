#ifndef _COWR_CURLWRAPPER_CURL

#define _COWR_CURLWRAPPER_CURL

#pragma GCC push_options
#pragma GCC optimize("O0")

#include <cassert>

#include <exception>

#include <curl/curl.h>

#include "CurlEasyHandle.hpp"
#include "CurlError.hpp"
#include "CurlMultiHandle.hpp"

class {
    struct todo {
        todo()
        {
            if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
                throw(CurlGlobalInitError());
        }
        ~todo()
        {
            curl_global_cleanup();
        }
    } _;
} __curl_initializer;

#pragma GCC pop_options

#endif
