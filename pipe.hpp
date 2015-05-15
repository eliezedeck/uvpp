#pragma once

/*
 * pipe.hpp
 *
 *  Created on: May 15, 2015 10:16:05 AM
 *      Author: Elie Zedeck RANDRIAMIANDRIRAY <rez@eliezedeck.com>
 */

#include "stream.hpp"
#include "loop.hpp"

#include <uv.h>

#include <iostream>

using namespace std;

namespace uvpp
{

/**
 * Pipe can be used for IPC over unix domain socket or anonymous pipes.
 *
 * Guidelines:
 * - When used with Process, make sure you read_start() only after the process
 *   has been started.
 */
class Pipe: public stream<uv_pipe_t>
{
public:
    Pipe(bool use_for_ipc = false)
            : stream<uv_pipe_t>()
    {
        uv_pipe_init(uv_default_loop(), get(), use_for_ipc);
    }

    Pipe(loop &l, bool use_for_ipc = false)
            : stream<uv_pipe_t>()
    {
        uv_pipe_init(l.get(), get(), use_for_ipc);
    }
};

} // namespace uvpp
