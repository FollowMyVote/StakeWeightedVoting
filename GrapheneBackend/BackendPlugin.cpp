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
#include "BackendPlugin.hpp"
#include "BackendServer.hpp"
#include "compat/FcStreamWrapper.hpp"

#include <capnp/rpc-twoparty.h>

namespace swv {

BackendPlugin::BackendPlugin()
    : tasks(errorLogger) {}
BackendPlugin::~BackendPlugin() noexcept {}

std::string BackendPlugin::plugin_name() const {
    return "Follow My Vote Backend";
}

void BackendPlugin::plugin_initialize(const boost::program_options::variables_map& options) {
    serverPort = options["port"].as<uint16_t>();
    KJ_LOG(INFO, "Follow My Vote plugin initialized");
}

void BackendPlugin::plugin_startup() {
    running = true;
    server.listen(serverPort);
    KJ_LOG(INFO, "Server is up", server.get_port());
    fc::async([this]{acceptLoop();});
}

void BackendPlugin::plugin_shutdown() {
    KJ_LOG(INFO, "Follow My Vote plugin shutting down");
    running = false;
    server.close();
    clients.clear();
}

void BackendPlugin::plugin_set_program_options(boost::program_options::options_description& command_line_options,
                                                    boost::program_options::options_description& config_file_options) {
    namespace bpo = boost::program_options;
    command_line_options.add_options()("port,p", bpo::value<uint16_t>()->default_value(17073),
                                       "The port for the server to listen on");
    config_file_options.add_options()("port,p", bpo::value<uint16_t>()->default_value(17073),
                                      "The port for the server to listen on");
}

struct BackendPlugin::ActiveClient {
    kj::Own<kj::AsyncIoStream> connection;
    capnp::TwoPartyVatNetwork network;
    capnp::RpcSystem<capnp::rpc::twoparty::VatId> rpcSystem;

    ActiveClient(capnp::Capability::Client bootstrapInterface,
                 kj::Own<kj::AsyncIoStream>&& connectionParam)
        : connection(kj::mv(connectionParam)),
          network(*connection, capnp::rpc::twoparty::Side::SERVER),
          rpcSystem(makeRpcServer(network, kj::mv(bootstrapInterface))) {}
};

void BackendPlugin::acceptLoop() {
    while (running) {
        try {
            auto client = kj::heap<fc::tcp_socket>();
            server.accept(*client);
            auto clientId = nextClientId++;
            KJ_LOG(INFO, "FMV client connected", std::string(client->remote_endpoint()), clientId);
            clients.emplace(std::make_pair(clientId, prepareClient(kj::mv(client))));
            tasks.add(clients[clientId]->network.onDisconnect().then([this, clientId] {
                KJ_LOG(INFO, "FMV client disconnected", clientId);
                clients.erase(clientId);
            }));
        } catch (kj::Exception e) {
            KJ_LOG(ERROR, "Exception while processing client", e);
        }
    }
}

kj::Own<BackendPlugin::ActiveClient> BackendPlugin::prepareClient(kj::Own<fc::tcp_socket> clientSocket) {
    // TODO: authenticate client, setup encryption, figure out how to clean up closed connections
    return kj::heap<BackendPlugin::ActiveClient>(kj::heap<BackendServer>(),
                                                 kj::heap<FcStreamWrapper>(kj::mv(clientSocket)));
}

} // namespace swv
