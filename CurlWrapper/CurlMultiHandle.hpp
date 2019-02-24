#ifndef _COWR_CURLWRAPPER_CURLMULTIHANDLE

#define _COWR_CURLWRAPPER_CURLMULTIHANDLE

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>

#include <curl/curl.h>

#include "CurlEasyHandle.hpp"
#include "CurlError.hpp"

class CurlMultiHandle {
public:
private:
    CURLM* multi_handle;
    std::set<std::shared_ptr<CurlEasyHandle>> easy_handles;
    std::map<CURL*, std::weak_ptr<CurlEasyHandle>> lookup;

public:
    CurlMultiHandle()
        : multi_handle(curl_multi_init())
    {
        if (!multi_handle)
            throw(CurlMultiInitError());
    }

    ~CurlMultiHandle()
    {
        for (auto easy_handle : easy_handles)
            curl_multi_remove_handle(multi_handle, *easy_handle);
        curl_multi_cleanup(multi_handle);
    }

    CURLMcode addHandle(std::shared_ptr<CurlEasyHandle>& easy_handle)
    {
        easy_handles.insert(easy_handle);
        lookup[*easy_handle] = easy_handle;
        return curl_multi_add_handle(multi_handle, *easy_handle);
    }

    CURLMcode removeHandle(std::shared_ptr<CurlEasyHandle>& easy_handle)
    {
        easy_handles.erase(easy_handle);
        lookup.erase(*easy_handle);
        return curl_multi_remove_handle(multi_handle, *easy_handle);
    }

    int perform()
    {
        int running;

        curl_multi_perform(multi_handle, &running);

        struct CURLMsg* m;

        decltype(easy_handles) to_remove;
        do {
            int msgq = 0;
            m = curl_multi_info_read(multi_handle, &msgq);
            if (m && (m->msg == CURLMSG_DONE)) {
                CURL* e = m->easy_handle;
                auto it = easy_handles.find(std::shared_ptr<CurlEasyHandle>(lookup[e]));
                (*it)->onRequestDone();
                to_remove.insert(*it);
                // iphandle = easy_handles.erase(iphandle);
            }

        } while (m);

        for (auto h : to_remove) {
            removeHandle(h);
        }

        return running;
    }
};

#endif
