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

#include "BackendServer.hpp"
#include "TwoPartyServer.hpp"

#include <kj/debug.h>
#include <kj/async-io.h>
#include <kj/async-unix.h>

#include <iostream>
#include <signal.h>

int main() {
    kj::UnixEventPort::captureSignal(SIGINT);

    TwoPartyServer server(kj::heap<BackendServer>());
    auto asyncIo = kj::setupAsyncIo();
    auto promise = asyncIo.provider->getNetwork().parseAddress("127.0.0.1", 2572).then(
                       [&server](kj::Own<kj::NetworkAddress> addr)
    {
        auto listener = addr->listen();
        std::cout << "Listening on port " << listener->getPort() << std::endl;
        return server.listen(kj::mv(listener));
    }).eagerlyEvaluate([](kj::Exception&& e) {
        KJ_LOG(ERROR, e);
        exit(1);
    });

    asyncIo.unixEventPort.onSignal(SIGINT).wait(asyncIo.waitScope);
    std::cout << "\nServer exiting.\n";
    return 0;
}
