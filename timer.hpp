#pragma once

#include "handle.hpp"
#include "loop.hpp"
#include "error.hpp"

namespace uvpp
{

class Timer: public handle<uv_timer_t>
{
public:
    Timer()
            : handle()
    {
        uv_timer_init(uv_default_loop(), get());
    }

    Timer(loop &l)
            : handle()
    {
        uv_timer_init(l.get(), get());
    }

    bool start(std::function<void()> callback, const uint64_t timeout,
            const uint64_t repeat)
    {
        callbacks::store(get()->data, internal::uv_cid_timer, callback);
        return uv_timer_start(get(),
                [](uv_timer_t* handle) {
                    callbacks::invoke<decltype(callback)>(handle->data, internal::uv_cid_timer);
                }, timeout, repeat);
    }

    bool stop()
    {
        return uv_timer_stop(get());
    }

    bool again()
    {
        return uv_timer_again(get());
    }
};

} // namespace uvpp
