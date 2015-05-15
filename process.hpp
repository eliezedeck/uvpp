#pragma once

/*
 * process.hpp
 *
 *  Created on: May 14, 2015 9:43:19 PM
 *      Author: Elie Zedeck RANDRIAMIANDRIRAY <rez@eliezedeck.com>
 */

#include "handle.hpp"
#include "loop.hpp"
#include "pipe.hpp"

#include <uv.h>

#include <cstring>
#include <iostream>

using namespace std;

namespace uvpp
{

class Process: public handle<uv_process_t>
{
public:
    enum {
        STDIN = 0,
        STDOUT,
        STDERR
    };

    Process(loop &l)
            : handle<uv_process_t>(), l(l)
    {
        options = {0};
        ::memset(&stdios, 0, 3 * sizeof(uv_stdio_container_t));
        options.stdio = stdios;
        options.stdio_count = 3;
    }

    virtual ~Process()
    {
//        if (m_pipe_stdout) {
//            m_pipe_stdout->close([=]() {
//                cout << "Deleted STDOUT" << endl;
//                delete m_pipe_stdout;
//            });
//        }
//        if (m_pipe_stderr) {
//            m_pipe_stderr->close([=]() {
//                delete m_pipe_stderr;
//            });
//        }
        cout << "Process deleted" << endl;
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

    void
    prepare_to_read_output(int stdio)
    {
        switch (stdio) {
        case STDOUT:
            if (!m_pipe_stdout) {
                m_pipe_stdout = new Pipe(l);
            }
            options.stdio[1].flags = (uv_stdio_flags) (UV_CREATE_PIPE | UV_READABLE_PIPE);
            options.stdio[1].data.stream = (uv_stream_t*) m_pipe_stdout->get();
            break;
        case STDERR:
            if (!m_pipe_stderr) {
                m_pipe_stderr = new Pipe(l);
            }
            options.stdio[2].flags = (uv_stdio_flags) (UV_CREATE_PIPE | UV_READABLE_PIPE);
            options.stdio[2].data.stream = (uv_stream_t*) m_pipe_stderr->get();
            break;
        default:
            throw exception("Only STDOUT and STDERR supported for prepare_to_read_output()");
            break;
        }
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

    /**
     * start() launches the process for execution.
     *
     * Guidelines:
     * - Capturing outputs: set stdio flag to UV_CREATE_PIPE | UV_READABLE_PIPE for stdout/stderr
     * - Send inputs: set stdio flag to UV_CREATE_PIPE | UV_WRITABLE_PIPE for stdin
     * - read_start_output() ONLY after calling this start()
     *
     * Notes:
     * - There is no need to close() the Process because UV already does that when the process
     *   has been completed.
     */
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

    template<size_t max_alloc_size>
    bool
    read_start_output(int stdio, std::function<void(const char* buf, ssize_t len)> callback)
    {
        switch (stdio) {
        case STDOUT:
            assert(m_pipe_stdout != nullptr);
            m_pipe_stdout->read_start<max_alloc_size>(callback);
            break;
        case STDERR:
            assert(m_pipe_stderr != nullptr);
            m_pipe_stderr->read_start<max_alloc_size>(callback);
            break;
        default:
            throw exception("Only STDOUT and STDERR supported for read_start_output()");
            break;
        }
    }

    void tear_down_outputs()
    {
        if (m_pipe_stdout) {
            m_pipe_stdout->close([=]() {
                cout << "Deleted STDOUT" << endl;
                delete m_pipe_stdout;
            });
        }
        if (m_pipe_stderr) {
            m_pipe_stderr->close([=]() {
                delete m_pipe_stderr;
            });
        }
    }

private:
    loop &l;
    uv_stdio_container_t stdios[3];
    uv_process_options_t options;

    Pipe* m_pipe_stdout = nullptr;
    Pipe* m_pipe_stderr = nullptr;

    bool m_is_running = false;
};

} // namespace uvpp
