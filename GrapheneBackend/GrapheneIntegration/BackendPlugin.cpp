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
#include "VoteDatabase.hpp"
#include "ApiServers/BackendServer.hpp"
#include "compat/FcStreamWrapper.hpp"
#include <BotanIntegration/TlsPskAdaptorFactory.hpp>
#include <BotanIntegration/TlsPskAdaptor.hpp>

#include <contest.capnp.h>

#include <capnp/rpc-twoparty.h>

#include <graphene/utilities/key_conversion.hpp>

#include <fc/crypto/digest.hpp>

namespace swv {

BackendPlugin::BackendPlugin()
    : tasks(errorLogger) {}
BackendPlugin::~BackendPlugin() noexcept {}

std::string BackendPlugin::plugin_name() const {
    return "Follow My Vote Backend";
}

void BackendPlugin::plugin_initialize(const boost::program_options::variables_map& options) {
    serverPort = options["port"].as<uint16_t>();
    database = kj::heap<VoteDatabase>(*app().chain_database());
    database->registerIndexes();
    KJ_LOG(INFO, "Follow My Vote plugin initialized");
}

void BackendPlugin::plugin_startup() {
    database->startup(app().p2p_node());
    cryptoFactory = kj::heap<fmv::TlsPskAdaptorFactory>([&vdb = database](std::string clientName) {
        KJ_LOG(DBG, "Client authenticating", clientName);
        auto& index = vdb->db().get_index_type<gch::account_index>().indices().get<gch::by_name>();
        auto itr = index.find(clientName);
        KJ_REQUIRE(itr != index.end(), "Could not find client's account", clientName);

        auto privateKey = graphene::utilities::wif_to_key(
                              vdb->configuration().reader().getAuthenticatingKeyWif());
        KJ_REQUIRE(!!privateKey, "Authenticating key is not set!");
        auto secret = fc::digest(privateKey->get_shared_secret(itr->options.memo_key));
        return std::vector<uint8_t>(reinterpret_cast<uint8_t*>(secret.data()),
                                    reinterpret_cast<uint8_t*>(secret.data() + secret.data_size()));
    }, *CONTEST_PUBLISHING_ACCOUNT);

    running = true;
    server.set_reuse_address();
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

struct BackendPlugin::ClientConnection {
    BackendServer& server;
    kj::Own<kj::AsyncIoStream> connection;
    capnp::TwoPartyVatNetwork network;
    capnp::RpcSystem<capnp::rpc::twoparty::VatId> rpcSystem;

    ClientConnection(kj::Own<BackendServer> server,
                 kj::Own<kj::AsyncIoStream>&& connectionParam)
        : server(*server),
          connection(kj::mv(connectionParam)),
          network(*connection, capnp::rpc::twoparty::Side::SERVER),
          rpcSystem(makeRpcServer(network, kj::mv(server))) {}
};

void BackendPlugin::acceptLoop() {
    while (running) {
        try {
            auto client = kj::heap<fc::tcp_socket>();
            server.accept(*client);
            auto clientId = nextClientId++;
            KJ_LOG(INFO, "FMV client connected", std::string(client->remote_endpoint()), clientId);
            auto itr = clients.emplace(std::make_pair(clientId, prepareClient(kj::mv(client)))).first;
            tasks.add(itr->second->network.onDisconnect().then([this, clientId] {
                KJ_LOG(INFO, "FMV client disconnected", clientId);
                clients.erase(clientId);
            }));
        } catch (kj::Exception e) {
            KJ_LOG(ERROR, "Exception while processing client", e);
        }
    }
}

kj::Own<BackendPlugin::ClientConnection> BackendPlugin::prepareClient(kj::Own<fc::tcp_socket> clientSocket) {
    auto stream = cryptoFactory->addServerTlsAdaptor(kj::heap<FcStreamWrapper>(kj::mv(clientSocket)));
    return kj::heap<BackendPlugin::ClientConnection>(kj::heap<BackendServer>(*database), kj::mv(stream));
}

} // namespace swv
