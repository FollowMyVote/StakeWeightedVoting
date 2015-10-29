// Contents of this file are modified from an excerpt of the official Cap'nProto source code.
// Copyright (c) 2013-2014 Sandstorm Development Group, Inc. and contributors
// Licensed under the MIT License:
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef TWOPARTYCLIENT_HPP
#define TWOPARTYCLIENT_HPP

#include <capnp/rpc-twoparty.h>

class TwoPartyClient
{
    // Convenience class which implements a simple client.

public:
    explicit TwoPartyClient(kj::AsyncIoStream& connection);
    TwoPartyClient(kj::AsyncIoStream& connection, capnp::Capability::Client bootstrapInterface,
                   capnp::rpc::twoparty::Side side = capnp::rpc::twoparty::Side::CLIENT);

    capnp::Capability::Client bootstrap();
    // Get the server's bootstrap interface.

    inline kj::Promise<void> onDisconnect() { return network.onDisconnect(); }

private:
    capnp::TwoPartyVatNetwork network;
    capnp::RpcSystem<capnp::rpc::twoparty::VatId> rpcSystem;
};

#endif // TWOPARTYCLIENT_HPP
