#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "CurlWrapper/Curl.hpp"
#include "EventLoop.hpp"

struct QAQ {
    inline static int counter = 0;
    int x;
    QAQ()
    {
        std::cout << "C  " << ++counter << std::endl;
    }

    QAQ(QAQ&& r)
    {
        std::cout << "CM " << ++counter << std::endl;
    }

    QAQ(const QAQ& r)
    {
        std::cout << "CC " << ++counter << std::endl;
    }

    ~QAQ()
    {
        std::cout << "D  " << --counter << std::endl;
    }

    QAQ& operator=(const QAQ& r)
    {
        return *this;
    }
};

std::vector<char> readURL(std::string url)
{
    auto phandle = std::make_shared<CurlEasyHandle>();
    auto context = Context::getNowContext();
    auto main_context = Context::getMainContext();

    std::vector<char> buf;
    // std::clog << &buf << std::endl;

    phandle->setURL(url);
    phandle->setDataReceiveHandler(
        [&buf](void* buffer, size_t, size_t nmemb, CurlEasyHandle&) -> size_t {
            buf.insert(buf.end(),
                (char*)buffer,
                (char*)buffer + nmemb);

            return nmemb;
        });
    phandle->setRequestDoneHandler(
        [context](CurlEasyHandle&) -> void {
            Context::getNowContext()->Swap(*context);
        });

    EventLoop::getEventLoop().addCurlHandle(phandle);

    context->Swap(*main_context);

    return buf;
}

QAQ genQAQ() { return QAQ(); }

void readLOJ(int i)
{
    std::vector<char> data = cowr_await(std::bind(readURL, "loj.ac"));
    //cowr_await(std::bind(readURL, "loj.ac"));
    // cowr_await(LOL);
    // QAQ t = cowr_await(genQAQ);
    std::clog << "Done " << i << std::endl;
    std::cout.write(data.data(), data.size());
}

int main(void)
{
    EventLoop& loop = EventLoop::getEventLoop();
    // Context::getNowContext()->Call(readLOJ);

    // Context::getNowContext()->Call(readLOJ);
    //loop.loop(5000);

    // loop.callLater(0, []() -> void {
    //     Context::getNowContext()->Call(readLOJ);
    // });
    // readURL("");

    for (int i = 0; i < 100; i++) {
        Context::getNowContext()->Call(readLOJ, i);
    }

    // for (int i = 0; i < 10; i++) {
    //     loop.callLater(1000 * i, [i]() -> void { std::clog << i << std::endl; });
    // }

    // for (int i = 0; i < 100; i++) {
    //     auto phandle = std::make_shared<CurlEasyHandle>();
    //     phandle->setURL("loj.ac");
    //     phandle->setDataReceiveHandler(
    //         [](void* buffer, size_t, size_t nmemb, CurlEasyHandle&) -> size_t {
    //             return nmemb;
    //         });
    //     phandle->setRequestDoneHandler(
    //         [i](CurlEasyHandle&) -> void {
    //             std::clog << i << " Done" << std::endl;
    //         });
    //     loop.addCurlHandle(phandle);
    // }

    loop.loop(10000);
    /*
    loop.callLater(0, []() -> void { Context::getNowContext()->Call(readLOJ); });
    loop.callLater(0, []() -> void { Context::getNowContext()->Call(readLOJ); });
    loop.callLater(0, []() -> void { Context::getNowContext()->Call(readLOJ); });
    loop.callLater(0, []() -> void { Context::getNowContext()->Call(readLOJ); });

    loop.loop(5000);
    */
    return 0;
}
