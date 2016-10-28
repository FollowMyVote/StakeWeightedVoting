#include "BitsharesWalletBridge.hpp"
#include "Converters.hpp"

#include <Utilities.hpp>

#include <kj/debug.h>
#include <kj/vector.h>

#include <memory>

namespace swv { namespace bts {

BitsharesWalletBridge::BitsharesWalletBridge(const QString& serverName)
    : QWebSocketServer (serverName, QWebSocketServer::SslMode::NonSecureMode) {}

BitsharesWalletBridge::~BitsharesWalletBridge() {}

////////////////////////////// BEGIN BlockchainWalletServer implementation
using BWB = BitsharesWalletBridge;
class BWB::BlockchainWalletApiImpl : public BlockchainWallet::Server {
    std::unique_ptr<QWebSocket> connection;
    int64_t nextQueryId = 0;
    std::map<int64_t, kj::Own<kj::PromiseFulfiller<QJsonValue>>> pendingRequests;
    // Promise is not default constructible; set a failed promise now; we'll overwrite it in our constructor
    kj::ForkedPromise<uint64_t> contestPublisherIdPromise;
    uint64_t contestPublisherId = 0;

public:
    BlockchainWalletApiImpl(std::unique_ptr<QWebSocket> connection);
    virtual ~BlockchainWalletApiImpl() {}

protected:
    void checkConnection();
    kj::Promise<QJsonValue> beginCall(QString method, QJsonArray params, bool asNotification = false);
    void finishCall(QString message);

    kj::Promise<QString> setFeesAndBroadcastTransaction(QJsonArray operations);

    // BlockchainWallet::Server interface
    virtual ::kj::Promise<void> getCoinById(GetCoinByIdContext context) override;
    virtual ::kj::Promise<void> getCoinBySymbol(GetCoinBySymbolContext context) override;
    virtual ::kj::Promise<void> getAllCoins(GetAllCoinsContext context) override;
    virtual ::kj::Promise<void> unlockWallet(UnlockWalletContext) override;
    virtual ::kj::Promise<void> listMyAccounts(ListMyAccountsContext context) override;
    virtual ::kj::Promise<void> getBalance(GetBalanceContext context) override;
    virtual ::kj::Promise<void> getBalancesBelongingTo(GetBalancesBelongingToContext context) override;
    virtual ::kj::Promise<void> getContestById(GetContestByIdContext context) override;
    virtual ::kj::Promise<void> getDecisionRecordById(GetDecisionRecordByIdContext context) override;
    virtual ::kj::Promise<void> getDatagramByBalance(GetDatagramByBalanceContext context) override;
    virtual ::kj::Promise<void> publishDatagram(PublishDatagramContext context) override;
    virtual ::kj::Promise<void> transfer(TransferContext context) override;
    virtual ::kj::Promise<void> getSharedSecret(GetSharedSecretContext context) override;
};

BWB::BlockchainWalletApiImpl::BlockchainWalletApiImpl(std::unique_ptr<QWebSocket> connection)
    : connection(kj::mv(connection)), contestPublisherIdPromise(nullptr) {
    KJ_REQUIRE(!!this->connection, "Internal Error: Attempted to create Bitshares blockchain wallet server with null "
                                   "connection to a Bitshares wallet");
    KJ_REQUIRE(this->connection->state() == QAbstractSocket::SocketState::ConnectedState,
               "Internal Error: Attempted to create Bitshares blockchain wallet server with bad connection to a "
               "Bitshares wallet", this->connection->state());
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

    // Look up the contest publishing account; we'll need to know its ID to authenticate contests
    auto contestPublisherName = QString::fromStdString(*::CONTEST_PUBLISHING_ACCOUNT);
    contestPublisherIdPromise = beginCall("blockchain.getAccountByName",
                                          QJsonArray() << contestPublisherName).then(
                                    [this](QJsonValue response) -> uint64_t {
         auto account = response.toObject();
         contestPublisherId = account["id"].toString().replace("1.2.", QString::null).toULongLong();
         return contestPublisherId;
    }).fork();

    // Someday we can implment encrypted communication with the BTS wallet, which would be negotiated here.
}

void BWB::BlockchainWalletApiImpl::checkConnection() {
    if (!connection || connection->state() != QAbstractSocket::SocketState::ConnectedState)
        throw KJ_EXCEPTION(DISCONNECTED, "Connection to Bitshares wallet has failed.");
}

void BWB::BlockchainWalletApiImpl::finishCall(QString message) {
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

kj::Promise<QString> BWB::BlockchainWalletApiImpl::setFeesAndBroadcastTransaction(QJsonArray operations) {
    return beginCall("blockchain.getTransactionFees", QJsonArray() << operations).then([this](QJsonValue response) {
        KJ_LOG(DBG, "Broadcasting transaction", QJsonDocument(response.toObject()).toJson().data());
        return beginCall("wallet.broadcastTransaction", QJsonArray() << response.toArray());
    }).then([](QJsonValue response) {
        KJ_DBG(response.toString().toStdString());
        return response.toString();
    });
}

kj::Promise<QJsonValue> BWB::BlockchainWalletApiImpl::beginCall(QString method, QJsonArray params, bool asNotification) {
    checkConnection();

    QJsonObject call {
        {"jsonrpc", "2.0"},
        {"method",  method},
        {"params",  params},
        {"id",      qint64(nextQueryId)}
    };
    if (asNotification)
        call.remove("id");
    connection->sendTextMessage(QJsonDocument(call).toJson(QJsonDocument::JsonFormat::Compact));
    if (asNotification)
        return QJsonValue();

    // Create a new PendingRequest, consisting of query ID and fulfiller
    auto paf = kj::newPromiseAndFulfiller<QJsonValue>();
    pendingRequests.emplace(std::make_pair(nextQueryId++, kj::mv(paf.fulfiller)));
    return kj::mv(paf.promise);
}

void populateCoin(::Coin::Builder builder, QJsonObject coin) {
    builder.setId(coin["id"].toString().replace("1.3.", QString::null).toULongLong());
    builder.setCreator(coin["issuer"].toString().toStdString());
    builder.setName(coin["symbol"].toString().toStdString());
    builder.setPrecision(coin["precision"].toInt());
}

kj::Promise<void> BWB::BlockchainWalletApiImpl::getCoinById(GetCoinByIdContext context) {
    KJ_LOG(DBG, __FUNCTION__);
    auto id = QStringLiteral("1.3.%1").arg(context.getParams().getId());
    return beginCall("blockchain.getObjectById", QJsonArray() << id).then([context](QJsonValue response) mutable {
        populateCoin(context.initResults().initCoin(), response.toObject());
    });
}

kj::Promise<void> BWB::BlockchainWalletApiImpl::getCoinBySymbol(GetCoinBySymbolContext context) {
    KJ_LOG(DBG, __FUNCTION__);
    auto symbol = QString::fromStdString(context.getParams().getSymbol());
    return beginCall("blockchain.getAssetBySymbol",
                     QJsonArray() << symbol).then([context](QJsonValue response) mutable {
        populateCoin(context.initResults().initCoin(), response.toObject());
    });
}

kj::Promise<void> BWB::BlockchainWalletApiImpl::getAllCoins(GetAllCoinsContext context) {
    KJ_LOG(DBG, __FUNCTION__);
    return beginCall("blockchain.getAllAssets", {}).then([context](QJsonValue response) mutable {
        auto assets = response.toArray();
        auto coins = context.initResults().initCoins(assets.size());
        auto index = 0u;
        for (const auto& asset : assets)
            populateCoin(coins[index++], asset.toObject());
    });
}

kj::Promise<void> BWB::BlockchainWalletApiImpl::unlockWallet(UnlockWalletContext) {
    KJ_LOG(DBG, __FUNCTION__);
    return beginCall("wallet.unlockWallet", {}, true).then([](QJsonValue){});
}

kj::Promise<void> BWB::BlockchainWalletApiImpl::listMyAccounts(ListMyAccountsContext context) {
    KJ_LOG(DBG, __FUNCTION__);
    return beginCall("wallet.getMyAccounts", {}).then([this, context](QJsonValue response) mutable {
        auto accounts = response.toArray();
        auto results = context.initResults().initAccountNames(accounts.size());
        auto index = 0u;
        for (const auto& account : accounts) {
            results.set(index++, account.toString().toStdString());
        }
    });
}

kj::Promise<void> BWB::BlockchainWalletApiImpl::getBalance(GetBalanceContext context) {
    KJ_LOG(DBG, __FUNCTION__);
    auto balanceId = context.getParams().getId();
    auto accountId = QStringLiteral("1.2.%1").arg(balanceId.getAccountInstance());
    auto coinId = QStringLiteral("1.3.%1").arg(balanceId.getCoinInstance());
    return beginCall("blockchain.getAccountBalances",
                     QJsonArray() << accountId).then([context, coinId, balanceId](QJsonValue response) mutable {
        auto balances = response.toArray();
        auto balanceItr = std::find_if(balances.begin(), balances.end(), [coinId](const QJsonValue& balance ) {
            return balance.toObject()["type"].toString() == coinId;
        });
        KJ_REQUIRE(balanceItr != balances.end(), "No such balance");
        auto result = context.initResults().initBalance();
        result.setId(context.getParams().getId());
        result.setAmount(balanceItr->toObject()["amount"].toVariant().toLongLong());
        result.setType(balanceItr->toObject()["type"].toString().replace("1.3.", "").toULongLong());
        result.setCreationOrder(balanceId.getAccountInstance());
    });
}

kj::Promise<void> BWB::BlockchainWalletApiImpl::getBalancesBelongingTo(GetBalancesBelongingToContext context) {
    KJ_LOG(DBG, __FUNCTION__);
    auto owner = QString::fromStdString(context.getParams().getOwner());
    auto accountPromise = beginCall("blockchain.getAccountByName", QJsonArray() << owner);
    return beginCall("blockchain.getAccountBalances", QJsonArray() << owner)
            .then([context, accountPromise = kj::mv(accountPromise)](QJsonValue response) mutable {
        return accountPromise.then([context, response](QJsonValue accountValue) mutable {
            auto accountId = accountValue.toObject()["id"].toString();
            auto balances = response.toArray();
            auto results = context.initResults().initBalances(balances.size());
            auto index = 0u;
            for (const auto& balance : balances) {
                auto balanceObject = balance.toObject();
                auto result = results[index++];
                result.getId().setAccountInstance(accountId.replace("1.2.", "").toULongLong());
                result.getId().setCoinInstance(balanceObject["type"].toString().replace("1.3.", "").toULongLong());
                result.setAmount(balanceObject["amount"].toVariant().toULongLong());
                result.setType(result.getId().getCoinInstance());
                result.setCreationOrder(result.getId().getAccountInstance());
            }
        });
    });
}

kj::Promise<void> BWB::BlockchainWalletApiImpl::getContestById(GetContestByIdContext context) {
    KJ_LOG(DBG, __FUNCTION__, context.getParams());
    auto operationInstance = context.getParams().getId().getOperationId();
    auto operationId = QStringLiteral("1.11.%1").arg(operationInstance);

    // First of all, make sure we've resolved the contest publisher (most likely this promise is already resolved)
    return contestPublisherIdPromise.addBranch().then([this, operationId](uint64_t) {
        // Look up the operation which published the contest
        return beginCall("blockchain.getObjectById", QJsonArray() << operationId);
    }).then([this, context, operationInstance](QJsonValue response) mutable {
        // We've now got what should be a custom operation with a datagram containing the contest. Parse out the
        // contest and fulfill the request, doing all necessary validity checks and authentications along the way.
        auto customOp = response.toObject()["op"].toArray();
        // A graphene custom_operation has opcode 35. Check that this operation has that opcode.
        auto opCode = customOp[0].toInt();
        KJ_REQUIRE(opCode == 35, "Invalid contest ID references an operation with wrong opcode",
                   opCode, operationInstance);

        // Custom op must have been published by Follow My Vote
        auto decodedOp = customOp[1].toObject();
        KJ_REQUIRE(decodedOp["payer"].toString().replace("1.2.", QString::null).toULongLong() == contestPublisherId,
                   "Could not authenticate contest referenced by contest ID");

        // Go ahead and fire off a call to get the block (we need it for the timestamp)
        auto blockPromise = beginCall("blockchain.getBlockByHeight", QJsonArray() << response.toObject()["block_num"]);

        // Custom op should contain a serialized datagram in base64 encoding. See buildPublishOperation() in
        // GrapheneBackend/ContestCreatorServer.cpp to see how this operation was created.
        auto operationData = QByteArray::fromHex(decodedOp["data"].toString().toLocal8Bit());
        auto operationDataReader = convertBlob(operationData);
        KJ_REQUIRE(capnp::Data::Reader(operationDataReader.slice(0, ::VOTE_MAGIC->size())) == *::VOTE_MAGIC,
                   "Invalid contest ID references an operation which was not published by Follow My Vote software",
                   operationInstance);
        BlobMessageReader datagramMessage(operationDataReader.slice(::VOTE_MAGIC->size(), operationDataReader.size()));
        auto datagramReader = datagramMessage->getRoot<::Datagram>();
        auto key = datagramReader.getKey().getKey();
        KJ_REQUIRE(key.isContestKey(), "Invalid contest ID references a datagram which does not contain a contest",
                   operationInstance);
        auto result = context.initResults().initContest();

        // Set creator's signature, if present
        if (key.getContestKey().getCreator().isSignature())
            result.setSignature(key.getContestKey().getCreator().getSignature().getSignature());

        // Deserialize the contest from the datagram and set it in the results
        BlobMessageReader contestMessage(datagramReader.getContent());
        result.setValue(contestMessage->getRoot<::Contest>());

        return blockPromise;
    }).then([context](QJsonValue block) mutable {
        // Contests may have a start time of zero, which indicates that they start when published. If this is the case,
        // set the start time to the time of publication.
        auto contest = context.getResults().getContest().getValue();
        auto timestamp = static_cast<uint64_t>(QDateTime::fromString(block.toObject()["timestamp"].toString(),
                                               Qt::ISODate).toMSecsSinceEpoch());
        contest.setStartTime(std::max(contest.getStartTime(), timestamp));
    });
}

::kj::Promise<void> BWB::BlockchainWalletApiImpl::getDecisionRecordById(GetDecisionRecordByIdContext context) {
    KJ_LOG(DBG, __FUNCTION__, context.getParams());
    auto operationInstance = context.getParams().getId().getOperationId();
    context.getResults().getRecord().getId().setOperationId(operationInstance);
    auto operationId = QStringLiteral("1.11.%1").arg(operationInstance);
    return beginCall("blockchain.getObjectById", QJsonArray() << operationId).then(
                [this, context, operationInstance](QJsonValue response) mutable {
        // This parsing is pretty similar to that in getContest above. See there for commentary
        auto customOp = response.toObject()["op"].toArray();
        auto opCode = customOp[0].toInt();
        KJ_REQUIRE(opCode == 35, "Invalid decision ID references an operation with wrong opcode",
                   opCode, operationInstance);
        auto decodedOp = customOp[1].toObject();
        auto operationData = QByteArray::fromHex(decodedOp["data"].toString().toLocal8Bit());
        auto operationDataReader = convertBlob(operationData);
        KJ_REQUIRE(capnp::Data::Reader(operationDataReader.slice(0, ::VOTE_MAGIC->size())) == *::VOTE_MAGIC,
                   "Invalid decision ID references an operation which was not published by Follow My Vote software",
                   operationInstance);
        BlobMessageReader datagramMessage(operationDataReader.slice(::VOTE_MAGIC->size(), operationDataReader.size()));
        auto datagramReader = datagramMessage->getRoot<::Datagram>();
        auto key = datagramReader.getKey().getKey();
        KJ_REQUIRE(key.isDecisionKey(), "Invalid decision ID references a datagram which does not contain a decision",
                   operationInstance);

        // Set the actual decision in the result
        auto result = context.getResults().getRecord();
        BlobMessageReader contestMessage(datagramReader.getContent());
        result.setDecision(contestMessage->getRoot<::Decision>());

        auto contestId = QStringLiteral("1.11.%1").arg(result.getDecision().getContest().getOperationId());

        // Fire off calls to fetch the other necessary info, storing promises for when that's done
        auto promiseArray = kj::heapArrayBuilder<kj::Promise<void>>(3);
        // Set timestamp
        promiseArray.add(beginCall("blockchain.getBlockByHeight", QJsonArray() << response.toObject()["block_num"])
                .then([result](QJsonValue block) mutable {
            auto timestamp = static_cast<uint64_t>(QDateTime::fromString(block.toObject()["timestamp"].toString(),
                                                   Qt::ISODate).toMSecsSinceEpoch());
            result.setTimestamp(timestamp);
        }));
        // Set voter name
        promiseArray.add(beginCall("blockchain.getObjectById", QJsonArray() << decodedOp["payer"])
                .then([result](QJsonValue response) mutable {
            result.setVoter(response.toObject()["name"].toString().toStdString());
        }));
        // Set stake (fetch contest, get it's coin, find voter's balance in that coin)
        promiseArray.add(beginCall("blockchain.getObjectById", QJsonArray() << contestId).then(
        [this, result, decodedOp](QJsonValue response) mutable {
            // TODO: Be less optimistic here; check data validity like in getContestById
            // Share code between this and getContestById? Perhaps implement caching?
            auto data = QByteArray::fromHex(response.toObject()["op"].toArray()[1].toObject()["data"].toString().toLocal8Bit());
            auto datagram = convertSerialStruct<::Datagram>(data.mid(VOTE_MAGIC->size()));
            BlobMessageReader contestMessage(::Datagram::Reader(*datagram).getContent());
            auto coinInstance = contestMessage->getRoot<::Contest>().getCoin();

            return beginCall("blockchain.getAccountBalances", QJsonArray() << decodedOp["payer"]).then(
            [result, coinInstance] (QJsonValue response) mutable {
                // Set weight to 0, and overwrite it if we find a balance of the right type
                result.setWeight(0);
                auto balances = response.toArray();
                auto coinId = QStringLiteral("1.3.%1").arg(coinInstance);
                // I don't care whether I find it or not, I'm just using find_if so it stops iterating if we find it
                std::find_if(balances.begin(), balances.end(), [result, coinId](const QJsonValue& balance) mutable {
                    if (balance.toObject()["type"].toString() == coinId) {
                        result.setWeight(balance.toObject()["amount"].toString().toULongLong());
                        return true;
                    }
                    return false;
                });
            });
        }));

        // Return a promise for the subcalls to finish. When they're done, the result will be ready to return.
        return kj::joinPromises(promiseArray.finish());
    });
}

kj::Promise<void> BWB::BlockchainWalletApiImpl::getDatagramByBalance(GetDatagramByBalanceContext context) {
    KJ_LOG(DBG, __FUNCTION__, context.getParams());

    // Unfortunately, there's no good way to do this with a standard Bitshares wallet. Adding explicit support for FMV
    // to the wallet would let us track datagrams directly, but as it stands now, we just have to iterate through the
    // account's history looking for custom ops.
    auto accountId = QStringLiteral("1.2.%1").arg(context.getParams().getBalanceId().getAccountInstance());

    // Custom operation has opcode 35
    return beginCall("blockchain.getAccountHistoryByOpCode",
                     QJsonArray() << accountId << 35).then([this](QJsonValue response) mutable {
        auto operationIds = response.toArray();
        auto operations = kj::heapArrayBuilder<kj::Promise<QJsonObject>>(operationIds.size());
        for (auto op : operationIds) {
            operations.add(beginCall("blockchain.getObjectById", QJsonArray() << op).then([](QJsonValue response) {
                               return response.toObject();
                           // If any calls fail, just store a null object; we still want to process the rest of them
                           }, [](kj::Exception) { return QJsonObject(); }));
        }

        return kj::joinPromises(operations.finish());
    }).then([context](kj::Array<QJsonObject> operations) mutable {
        // Iterate the operations from most recent to oldest
        for (auto opObject : operations) {
            // Skip over any operations that aren't FMV custom ops
            if (opObject == QJsonObject() || opObject["op"].toArray()[0].toInt() != 35)
                continue;
            auto data = QByteArray::fromHex(opObject["op"].toArray()[1].toObject()["data"].toString().toLocal8Bit());
            if (!data.startsWith(convertBlob(*VOTE_MAGIC)))
                continue;

            // Deserialize the datagram
            auto datagram = convertSerialStruct<::Datagram>(data.mid(VOTE_MAGIC->size()));
            ::Datagram::Reader reader = *datagram;

            // If the datagram's key matches our search key, finish the call
            if (reader.getKey() == context.getParams().getKey()) {
                context.getResults().setDatagram(reader);
                return;
            }
        }

        KJ_FAIL_REQUIRE("Could not find requested datagram", context.getParams());
    });
}

kj::Promise<void> BWB::BlockchainWalletApiImpl::publishDatagram(PublishDatagramContext context) {
    KJ_LOG(DBG, __FUNCTION__, context.getParams());

    auto publisherId = QStringLiteral("1.2.%1").arg(context.getParams().getPublishingBalance().getAccountInstance());
    auto payerId = QStringLiteral("1.2.%1").arg(context.getParams().getPayingBalance().getAccountInstance());

    ReaderPacker packer(context.getParams().getDatagram());
    auto buffer = convertBlob(*VOTE_MAGIC) + convertBlob(packer.array());

    auto customOp = QJsonObject{
        // The opcode for a custom operation is 35
        {"code", 35},
        {"op", QJsonObject{
             {"payer", payerId},
             {"required_auths", publisherId == payerId? QJsonArray() : QJsonArray() << publisherId},
             {"id", 0},
             {"data", QString(buffer.toHex())}
         }}
    };
    return setFeesAndBroadcastTransaction(QJsonArray() << customOp).then([](QString){});
}

kj::Promise<void> BWB::BlockchainWalletApiImpl::transfer(TransferContext context) {
    KJ_LOG(DBG, __FUNCTION__, context.getParams());

    auto senderNameOrId = QString::fromStdString(context.getParams().getSendingAccount());
    auto recipientNameOrId = QString::fromStdString(context.getParams().getReceivingAccount());

    auto transferOp = kj::heap<QJsonObject>({
        // The opcode for a transfer is 0
        {"code", 0},
        {"op", QJsonObject{
             {"from", senderNameOrId},
             {"to", recipientNameOrId},
             {"amount", QJsonObject {
                  {"amount", qint64(context.getParams().getAmount())},
                  {"asset_id", QStringLiteral("1.3.%1").arg(context.getParams().getCoinId())}
              }},
             // The memo is just a nonce; it has no real meaning, so there's no value in encrypting it. Leave the keys
             // null.
             {"memo", QJsonObject {
                  {"message", QString::fromStdString(context.getParams().getMemo())},
                  {"from", "BTS1111111111111111111111111111111114T1Anm"},
                  {"to", "BTS1111111111111111111111111111111114T1Anm"},
                  {"nonce", 0}
              }}
         }}
    });

    // We may need to look up some things. If so, store the promises in this vector
    auto lookup = [this, &transferOp] (QString name, QString toOrFrom) {
        return beginCall("blockchain.getAccountByName",
                               QJsonArray() << name).then(
                         [op = transferOp.get(), name, toOrFrom](QJsonValue result) {
            // Changing a nested QJsonObject is trickier than it sounds, but this works:
            // Make a copy of the nested object, change it, and overwrite it in the outer object
            auto copy = op->value("op").toObject();
            copy[toOrFrom] = result.toObject()["id"].toString();
            op->insert("op", copy);
        });
    };
    kj::Vector<kj::Promise<void>> promises;
    if (senderNameOrId[0].isLetter())
        // Sender is a name, not an ID. Look up the ID and store it in the op
        promises.add(lookup(senderNameOrId, "from"));
    if (recipientNameOrId[0].isLetter())
        // Same as with sender: look up ID
        promises.add(lookup(recipientNameOrId, "to"));

    // Wait for all promises in the vector to resolve; at that point, transferOp will be ready.
    return kj::joinPromises(promises.releaseAsArray()).then([this, op = kj::mv(transferOp)] {
        return setFeesAndBroadcastTransaction(QJsonArray() << *op).then([](QString){});
    });
}

::kj::Promise<void> BWB::BlockchainWalletApiImpl::getSharedSecret(GetSharedSecretContext context) {
    KJ_LOG(DBG, __FUNCTION__, context.getParams());

    auto myNameOrId = QString::fromStdString(context.getParams().getMyAccountNameOrId());
    auto otherNameOrId = QString::fromStdString(context.getParams().getOtherAccountNameOrId());

    return beginCall("wallet.getSharedSecret",
                     QJsonArray() << myNameOrId << otherNameOrId).then([this, context](QJsonValue response) mutable {
        auto secret = QByteArray::fromHex(response.toString().toLocal8Bit());
        context.getResults().setSecret(convertBlob(secret));
    });
}
////////////////////////////// END BlockchainWalletServer implementation

kj::Promise<BlockchainWallet::Client> BitsharesWalletBridge::nextWalletClient() {
    while (hasPendingConnections()) {
        auto socket = std::unique_ptr<QWebSocket>(nextPendingConnection());
        if (socket->state() != QAbstractSocket::ConnectedState) {
            KJ_LOG(WARNING, "Discarding already-closed socket to wallet", socket->state());
            continue;
        }

        connect(socket.get(), &QWebSocket::disconnected, this, &BitsharesWalletBridge::connectionLost);
        auto server = kj::heap<BlockchainWalletApiImpl>(kj::mv(socket));
        return BlockchainWallet::Client(kj::mv(server));
    }

    auto paf = kj::newPromiseAndFulfiller<BlockchainWallet::Client>();
    contexts.emplace(std::make_pair(nextContextId, ClientContext{QMetaObject::Connection(), kj::mv(paf.fulfiller)}));
    contexts[nextContextId].connection = connect(this, &QWebSocketServer::newConnection,
                                                 [this, contextId = nextContextId]() mutable {
        auto context = kj::mv(contexts[contextId]);
        contexts.erase(contextId);
        disconnect(context.connection);
        KJ_LOG(DBG, "Fulfilling promise for a BlockchainWallet client");
        std::unique_ptr<QWebSocket> socket(nextPendingConnection());
        connect(socket.get(), &QWebSocket::disconnected, this, &BitsharesWalletBridge::connectionLost);
        qDebug() << "Connection from" << socket->peerName() << "at"
                 << socket->peerAddress() << ":" << socket->peerPort();
        context.fulfiller->fulfill(kj::heap<BlockchainWalletApiImpl>(kj::mv(socket)));
    });
    ++nextContextId;
    KJ_LOG(DBG, "Promising a BlockchainWallet client");
    return kj::mv(paf.promise);
}

} } // namespace swv::bts
