#ifndef _COWR_EVENTLOOP

#define _COWR_EVENTLOOP

#pragma GCC push_options
#pragma GCC optimize("O0")

#include <stddef.h>

#include <sys/time.h>
#include <unistd.h>

#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <set>

#include "Coroutine.hpp"
#include "CurlWrapper/Curl.hpp"

using millisecond_t = long long;

millisecond_t getNowTime()
{
    struct timeval time;
    gettimeofday(&time, NULL);

    return time.tv_sec * 1000 + time.tv_usec / 1000;
}

class Event {

    uint64_t UUID_h, UUID_l;

public:
    millisecond_t time;
    std::function<void()> callback;

    Event(millisecond_t time, std::function<void()> callback)
        : time(time)
        , callback(callback)
        , UUID_h(rand())
        , UUID_l(rand())
    {
    }

    friend bool operator<(const Event& l, const Event& r)
    {
        if (l.time != r.time)
            return l.time < r.time;
        else if (l.UUID_h != r.UUID_h)
            return l.UUID_h < r.UUID_h;
        else
            return l.UUID_l < r.UUID_l;
    }

    friend bool operator==(const Event& l, const Event& r)
    {
        return l.UUID_l == r.UUID_l && l.UUID_h == r.UUID_h;
    }

    struct cmp_shared_ptr {
        bool operator()(const std::shared_ptr<Event>& l, const std::shared_ptr<Event>& r) const
        {
            if (l->time != r->time)
                return l->time < r->time;
            else if (l->UUID_h != r->UUID_h)
                return l->UUID_h < r->UUID_h;
            else
                return l->UUID_l < r->UUID_l;
        }
    };
};

class EventLoop {
private:
    std::set<std::shared_ptr<Event>, Event::cmp_shared_ptr> events;
    CurlMultiHandle curl_multi_handle;

    EventLoop() {}

public:
    static EventLoop& getEventLoop()
    {
        static EventLoop global_event_loop;
        return global_event_loop;
    }

    int addCurlHandle(std::shared_ptr<CurlEasyHandle> easy_handle)
    {
        return curl_multi_handle.addHandle(easy_handle);
    }

    std::shared_ptr<Event> callLater(millisecond_t delay, std::function<void()> callback)
    {
        return *(events.insert(std::make_shared<Event>(getNowTime() + delay, callback)).first);
    }

    void loop(millisecond_t delay)
    {
        millisecond_t end_time = getNowTime() + delay;

        while (delay < 0 || (getNowTime() < end_time)) {
            int running = 0;

            running = curl_multi_handle.perform();

            while (events.size() > 0 && (*events.begin())->time < getNowTime()) {
                auto it = events.begin();
                (*it)->callback();
                events.erase(it);

                running = 1;
            }

            if (!running)
                usleep(5000);
        }
    }
};

#pragma GCC pop_options

#endif
