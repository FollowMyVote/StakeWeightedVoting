/*
 * Copyright 2015 Follow My Vote, Inc.
 * This file is part of The Follow My Vote Stake-Weighted Voting Application ("SWV").
 *
 * SWV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SWV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SWV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "FcEventPort.hpp"

#include <fc/thread/thread.hpp>

namespace swv {

void FcEventPort::processKjEvents()
{
    if (kjLoop)
        kjLoop->run();

    if (isRunnable)
        fc::async([this]{processKjEvents();});
}

FcEventPort::~FcEventPort()
{}

bool FcEventPort::wait() {
    fc::usleep(fc::microseconds(1));
    return false;
}

bool FcEventPort::poll() {
    // Yield so other fc events can be processed
    fc::yield();
    return false;
}

void FcEventPort::setRunnable(bool runnable) {
    isRunnable = runnable;

    if (runnable)
        // Schedule the kj events to be processed
        fc::async([this]{processKjEvents();});
}

} // namespace swv
