#include "BitsharesWalletAdaptor.hpp"

#include <kj/debug.h>

namespace swv { namespace bts {

BitsharesWalletAdaptor::BitsharesWalletAdaptor(const QString& serverName)
    : QWebSocketServer (serverName, QWebSocketServer::SslMode::NonSecureMode) {}

BitsharesWalletAdaptor::~BitsharesWalletAdaptor() {}

////////////////////////////// BEGIN BlockchainWalletServer implementation
using BWA = BitsharesWalletAdaptor;
class BWA::BlockchainWalletServer : public BlockchainWallet::Server {
    std::unique_ptr<QWebSocket> connection;
    int64_t nextQueryId = 0;
    std::map<int64_t, kj::Own<kj::PromiseFulfiller<QJsonValue>>> pendingRequests;

public:
    BlockchainWalletServer(std::unique_ptr<QWebSocket> connection);
    virtual ~BlockchainWalletServer() {}

protected:
    void checkConnection();
    kj::Promise<QJsonValue> beginCall(QString method, QJsonArray params);
    void finishCall(QString message);

    // BlockchainWallet::Server interface
    virtual ::kj::Promise<void> getCoinById(GetCoinByIdContext context) override;
    virtual ::kj::Promise<void> getCoinBySymbol(GetCoinBySymbolContext context) override;
    virtual ::kj::Promise<void> getAllCoins(GetAllCoinsContext context) override;
    virtual ::kj::Promise<void> listMyAccounts(ListMyAccountsContext context) override;
    virtual ::kj::Promise<void> getBalance(GetBalanceContext context) override;
    virtual ::kj::Promise<void> getBalancesBelongingTo(GetBalancesBelongingToContext context) override;
    virtual ::kj::Promise<void> getContestById(GetContestByIdContext context) override;
    virtual ::kj::Promise<void> getDatagramByBalance(GetDatagramByBalanceContext context) override;
    virtual ::kj::Promise<void> publishDatagram(PublishDatagramContext context) override;
    virtual ::kj::Promise<void> transfer(TransferContext context) override;
};

BWA::BlockchainWalletServer::BlockchainWalletServer(std::unique_ptr<QWebSocket> connection)
    : connection(kj::mv(connection)) {
    KJ_REQUIRE(this->connection && this->connection->state() == QAbstractSocket::SocketState::ConnectedState,
               "Internal Error: Attempted to create Bitshares blockchain wallet server with no connection to a "
               "Bitshares wallet");
    connect(this->connection.get(), &QWebSocket::textMessageReceived, [this](QString message) {
        finishCall(message);
    });
    connect(this->connection.get(), &QWebSocket::disconnected, [this] {
        if (!pendingRequests.empty()) {
            KJ_LOG(WARNING, "Connection to Bitshares wallet lost while requests pending", pendingRequests.size());
            while (!pendingRequests.empty()) {
                pendingRequests.begin()->second->reject(KJ_EXCEPTION(DISCONNECTED,
                                                                     "Connection to Bitshares wallet lost"));
                pendingRequests.erase(pendingRequests.begin());
            }
        }
    });

    // Someday we can implment encrypted communication with the BTS wallet, which would be negotiated here.
}

void BWA::BlockchainWalletServer::checkConnection() {
    if (!connection || connection->state() != QAbstractSocket::SocketState::ConnectedState)
        throw KJ_EXCEPTION(DISCONNECTED, "Connection to Bitshares wallet has failed.");
}

void BWA::BlockchainWalletServer::finishCall(QString message) {
    QJsonParseError error;
    auto response = QJsonDocument::fromJson(message.toLocal8Bit(), &error).object();

    if (error.error != QJsonParseError::ParseError::NoError || !response.contains("id") ||
            (!response.contains("error") && !response.contains("result"))) {
        KJ_LOG(ERROR, "Got unrecognizeable message back from Bitshares wallet", message.toStdString());
        return;
    }

    auto itr = pendingRequests.find(response["id"].toVariant().toLongLong());
    if (itr == pendingRequests.end()) {
        KJ_LOG(ERROR, "Got response from Bitshares wallet, but the ID doesn't match any outstanding call",
               message.toStdString());
        return;
    }

    if (response.contains("result"))
        itr->second->fulfill(response["result"]);
    else
        itr->second->reject(KJ_EXCEPTION(FAILED,
                                         QString(QJsonDocument(response["error"].toObject()).toJson()).toStdString()));
    pendingRequests.erase(itr);
}

kj::Promise<QJsonValue> BWA::BlockchainWalletServer::beginCall(QString method, QJsonArray params) {
    checkConnection();

    QJsonObject call {
        {"jsonrpc", "2.0"},
        {"method",  method},
        {"params",  params},
        {"id",      nextQueryId}
    };
    connection->sendTextMessage(QJsonDocument(call).toJson(QJsonDocument::JsonFormat::Compact));

    // Create a new PendingRequest, consisting of
    auto paf = kj::newPromiseAndFulfiller<QJsonValue>();
    pendingRequests.emplace(std::make_pair(nextQueryId++, kj::mv(paf.fulfiller)));
    return kj::mv(paf.promise);
}

kj::Promise<void> BWA::BlockchainWalletServer::getCoinById(GetCoinByIdContext context) {
    return beginCall({}, {}).then([](auto){});
}

kj::Promise<void> BWA::BlockchainWalletServer::getCoinBySymbol(GetCoinBySymbolContext context) {
    return beginCall({}, {}).then([](auto){});
}

kj::Promise<void> BWA::BlockchainWalletServer::getAllCoins(GetAllCoinsContext context) {
    return beginCall({}, {}).then([](auto){});
}

kj::Promise<void> BWA::BlockchainWalletServer::listMyAccounts(ListMyAccountsContext context) {
    return beginCall({}, {}).then([](auto){});
}

kj::Promise<void> BWA::BlockchainWalletServer::getBalance(GetBalanceContext context) {
    return beginCall({}, {}).then([](auto){});
}

kj::Promise<void> BWA::BlockchainWalletServer::getBalancesBelongingTo(GetBalancesBelongingToContext context) {
    return beginCall({}, {}).then([](auto){});
}

kj::Promise<void> BWA::BlockchainWalletServer::getContestById(GetContestByIdContext context) {
    return beginCall({}, {}).then([](auto){});
}

kj::Promise<void> BWA::BlockchainWalletServer::getDatagramByBalance(GetDatagramByBalanceContext context) {
    return beginCall({}, {}).then([](auto){});
}

kj::Promise<void> BWA::BlockchainWalletServer::publishDatagram(PublishDatagramContext context) {
    return beginCall({}, {}).then([](auto){});
}

kj::Promise<void> BWA::BlockchainWalletServer::transfer(TransferContext context) {
    return beginCall({}, {}).then([](auto){});
}
////////////////////////////// END BlockchainWalletServer implementation

BlockchainWallet::Client BitsharesWalletAdaptor::nextWalletClient() {
    KJ_REQUIRE(hasPendingConnections(), "Cannot get next wallet client without any pending connections");
    return kj::heap<BlockchainWalletServer>(std::unique_ptr<QWebSocket>(nextPendingConnection()));
}

} } // namespace swv::bts
