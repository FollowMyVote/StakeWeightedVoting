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
#ifndef FCEVENTPORT_HPP
#define FCEVENTPORT_HPP

#include <kj/async.h>

namespace swv {

class FcEventPort : public kj::EventPort
{
    // Simple EventPort implementation to allow a KJ event loop to run in a thread scheduled by an FC event loop
    // Make sure to call setLoop with a pointer to the KJ event loop which will be sharing this thread as soon as
    // possible after construction.

    bool isRunnable = false;
    kj::EventLoop* kjLoop = nullptr;

    void processKjEvents();

public:
    virtual ~FcEventPort();

    void setLoop(kj::EventLoop* kjLoop) {
        // Store a pointer to the KJ event loop to call run() on when it needs to process events. The FcEventPort does
        // not take ownership of kjLoop. Make sure to call setLoop(nullptr) or destroy the FcEvenPort prior to
        // deallocating kjLoop.
        this->kjLoop = kjLoop;
    }

    // EventPort interface
    virtual bool wait() override;
    virtual bool poll() override;
    virtual void setRunnable(bool runnable) override;
};

} // namespace swv

#endif // FCEVENTPORT_HPP
