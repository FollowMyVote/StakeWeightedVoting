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

#include "TwoPartyServer.hpp"

#include <kj/debug.h>

namespace swv {

TwoPartyServer::TwoPartyServer(capnp::Capability::Client bootstrapInterface)
    : bootstrapInterface(kj::mv(bootstrapInterface)), tasks(*this) {}

TwoPartyServer::~TwoPartyServer() {}

struct TwoPartyServer::AcceptedConnection {
  kj::Own<kj::AsyncIoStream> connection;
  capnp::TwoPartyVatNetwork network;
  capnp::RpcSystem<capnp::rpc::twoparty::VatId> rpcSystem;

  explicit AcceptedConnection(capnp::Capability::Client bootstrapInterface,
                              kj::Own<kj::AsyncIoStream>&& connectionParam)
      : connection(kj::mv(connectionParam)),
        network(*connection, capnp::rpc::twoparty::Side::SERVER),
        rpcSystem(makeRpcServer(network, kj::mv(bootstrapInterface))) {}
};

void TwoPartyServer::accept(kj::Own<kj::AsyncIoStream>&& connection) {
  auto connectionState = kj::heap<AcceptedConnection>(bootstrapInterface, kj::mv(connection));

  // Run the connection until disconnect.
  auto promise = connectionState->network.onDisconnect().then([]{KJ_DBG("Disconnected");});
  tasks.add(promise.attach(kj::mv(connectionState)));
}

kj::Promise<void> TwoPartyServer::listen(kj::Own<kj::ConnectionReceiver> listener) {
  auto promise = listener->accept();
  return promise.then(kj::mvCapture(kj::mv(listener),
                          [this](kj::Own<kj::ConnectionReceiver> listener,
                                 kj::Own<kj::AsyncIoStream>&& connection) mutable {
    accept(kj::mv(connection));
    return listen(kj::mv(listener));
  }));
}

void TwoPartyServer::taskFailed(kj::Exception&& exception) {
  KJ_LOG(ERROR, exception);
}

} // namespace swv
