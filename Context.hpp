#ifndef _COWR_CONTEXT

#define _COWR_CONTEXT

#include <exception>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include <stddef.h>
#include <ucontext.h>

class Context;

inline std::exception_ptr now_exception;
inline bool exception_occured;
inline bool is_yield;

inline Context* now_context;
inline int now_context_id = 1;

class Context {
private:
    inline static std::map<int, Context*> contexts;
    inline static int counter = 1;

    int id;
    int prev_id, succ_id;
    size_t size;
    unsigned char *stack, *wth_tls_fs28;
    std::function<void(void)> call_func;

    std::weak_ptr<Context> self;

    ucontext_t context;

    Context()
        : size(0)
        , stack(nullptr)
        , id(counter++)
    {
        contexts[id] = this;
    }

public:
    Context(const Context& c)
        = delete;

    ~Context()
    {
        // std::clog << "~" << id << std::endl;
        if (stack) {
            delete[] stack;
            // delete[] wth_tls_fs28;
        }
        contexts.erase(id);
    }

    void setSelf(std::shared_ptr<Context>& shared_self)
    {
        self = shared_self;
    }

    void Save()
    {
        getcontext(&context);
    }

    void Load()
    {
        setcontext(&context);
    }

    void Make(std::function<void(void)> func,
        std::vector<std::shared_ptr<Context>> list,
        size_t stack_size = 4096,
        Context* link = nullptr)
    {
        if (stack) {
            delete[] stack;
            // delete[] wth_tls_fs28;
        }

        size = stack_size;
        stack = new unsigned char[stack_size];
        // wth_tls_fs28 = new unsigned char[stack_size];

        // std::clog << id << ' ' << (void*)stack << ' ' << (void*)(stack + stack_size) << std::endl;

        call_func = func;

        getcontext(&context);
        context.uc_stack.ss_flags = 0;
        context.uc_stack.ss_sp = stack;
        context.uc_stack.ss_size = stack_size;
        context.uc_link = link ? &(link->context) : NULL;
        makecontext(&context,
            (void (*)(void))(CallFunctionVV),
            5,
            &call_func,
            new std::vector<std::shared_ptr<Context>> { list },
            link ? link->getID() : 0);
    }

    template <size_t stack_size = 8192, typename Callable, typename... Args>
    void Call(Callable func, Args... args)
    {
        auto next = Context::makeShared();
        next->setSelf(next);

        next->Make(std::bind(func, args...),
            std::vector<std::shared_ptr<Context>> {
                std::shared_ptr<Context>(self),
                next },
            stack_size,
            this);

        next->prev_id = id;
        succ_id = next->id;
        now_context_id = next->id;
        now_context = &(*next);

        swapcontext(&context, &next->context);

        now_context_id = id;
        now_context = this;
    }

    void Yield()
    {
        auto it = contexts[prev_id];
        now_context_id = it->id;
        now_context = it;
        is_yield = true;
        exception_occured = false;

        swapcontext(&context, &(it->context));
    }

    void Resume()
    {
        auto it = contexts[succ_id];
        now_context_id = it->id;
        now_context = it;

        swapcontext(&context, &(it->context));
    }

    void Swap(Context& r)
    {
        now_context_id = r.id;
        now_context = &r;

        swapcontext(&context, &(r.context));
    }

    int getID()
    {
        return id;
    }

    operator ucontext_t()
    {
        return context;
    }

    static Context* getNowContext()
    {
        // auto it = contexts.find(now_context_id);
        // if (it == contexts.end())
        //     return nullptr;
        // else
        //     return it->second;
        return now_context;
    }

    static Context* getMainContext()
    {
        return contexts[1];
    }

    static std::shared_ptr<Context> makeShared()
    {
        return std::shared_ptr<Context>(new Context);
    }

    static void CallFunctionVV(std::function<void(void)>* func,
        std::vector<std::shared_ptr<Context>>* list, int next_id)
    {

        // std::clog << now_context_id << ' ' << &next_id << std::endl;
        try {
            std::vector<std::shared_ptr<Context>> _ = *list;

            (*func)();

            now_context_id = next_id;

            delete list;

            exception_occured = false;
        } catch (...) {
            now_exception = std::current_exception();
            exception_occured = true;
        }

        is_yield = false;
    }
};

class {
    std::shared_ptr<Context> global_context = Context::makeShared();

    int _ = [this]() -> int {
        global_context->setSelf(global_context);
        now_context = &(*global_context);
        return 0;
    }();
} __global_context_initializer;

#endif
