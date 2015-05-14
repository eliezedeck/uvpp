#pragma once

/*
 * process.hpp
 *
 *  Created on: May 14, 2015 9:43:19 PM
 *      Author: Elie Zedeck RANDRIAMIANDRIRAY <rez@eliezedeck.com>
 */


#include "handle.hpp"
#include "loop.hpp"

#include <uv.h>

#include <iostream>


using namespace std;


namespace uvpp {

class Process : public handle<uv_process_t>
{
public:
    Process(loop &l) :
        handle<uv_process_t>(),
        l(l)
    {
        options = {0};
    }

    uv_process_options_t&
    get_options()
    {
        return options;
    }

    void
    set_options(uv_process_options_t o)
    {
        options = o;
    }

    int
    kill(int sig=SIGTERM)
    {
        if (m_is_running) {
            uv_process_kill(get(), sig);
            // The Callback will be called after this has been killed.
            // Thus, the `term_signal` will contain the signal.
        }
    }

    bool
    start(std::function<void(int64_t exit_status, int term_signal)> callback) {
        m_is_running = true;

        callbacks::store(get()->data, internal::uv_cid_process_exit, callback, this);
        options.exit_cb = [](uv_process_t* handle, int64_t exit_status, int term_signal) {
            if (auto proc = reinterpret_cast<Process*>(callbacks::get_data<decltype(callback)>(handle->data, internal::uv_cid_process_exit))) {
                proc->m_is_running = false;
                proc->close();
            }
            callbacks::invoke<decltype(callback)>(handle->data, internal::uv_cid_process_exit, exit_status, term_signal);
        };

        return uv_spawn(l.get(), get(), &options);
    }

private:
    loop &l;
    uv_process_options_t options;

    bool m_is_running = false;
};

} // namespace uvpp
