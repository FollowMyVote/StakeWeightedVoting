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

#include "BlockchainWalletApi.hpp"
#include "DataStructures/Coin.hpp"
#include "DataStructures/Balance.hpp"
#include "Converters.hpp"
#include "PromiseConverter.hpp"

#include <Utilities.hpp>

#include <kj/debug.h>

namespace swv {

const static QString PERSISTENT_DECISION_KEY = QStringLiteral("persistedDecisions/%1");

BlockchainWalletApi::BlockchainWalletApi(PromiseConverter& promiseConverter, QObject *parent)
    : QObject(parent),
      promiseConverter(promiseConverter),
      m_chain(KJ_EXCEPTION(FAILED, "Blockchain client has not been set yet!"))
{}

BlockchainWalletApi::~BlockchainWalletApi() noexcept
{}

void BlockchainWalletApi::setChain(BlockchainWallet::Client chain) {
    this->m_chain = chain;
}

QJSValue BlockchainWalletApi::getDecision(QString owner, QString contestId) {
    auto promise = _getDecision(kj::mv(owner), kj::mv(contestId));
    return promiseConverter.convert(kj::mv(promise), [](swv::data::Decision* d) -> QVariant {
        return {QVariant::fromValue<QObject*>(d)};
    });
}

kj::Promise<data::Decision*> BlockchainWalletApi::_getDecision(QString owner, QString contestId) {
    using Reader = ::Balance::Reader;
    using DatagramResponse = capnp::Response<BlockchainWallet::GetDatagramByBalanceResults>;

    // First fetch the contest object, and all balances belonging to this owner so we can search those balances for
    // datagrams pertaining to the contest
    auto promise = getContestImpl(contestId).then([=](capnp::Response<BlockchainWallet::GetContestByIdResults> r) {
        return getBalancesBelongingToImpl(owner).then([contestResponse = kj::mv(r)](auto response) mutable {
            return std::make_tuple(kj::mv(contestResponse), kj::mv(response));
        });
    }).then([=](auto contestAndBalances) {
        ::Signed<Contest>::Reader contest = std::get<0>(contestAndBalances).getContest();
        auto tmpBalances = std::get<1>(contestAndBalances).getBalances();
        kj::Array<::Balance::Reader> balances = kj::heapArray<::Balance::Reader>(tmpBalances.begin(),
                                                                                 tmpBalances.end());

        // Filter out all balances which are not in the coin this contest is weighted in; only balances in the coin the
        // contest is weighted in can have valid decisions on that contest
        {
            auto newEnd = std::remove_if(balances.begin(), balances.end(), [contest](Reader balance) {
                 return balance.getType() != contest.getValue().getCoin();
            });
            KJ_REQUIRE(newEnd != balances.begin(), "No balances found in the contest's coin, so no decision exists.");
            balances = kj::heapArray<::Balance::Reader>(balances.begin(), newEnd);
        }
        KJ_DBG("Looking for decisions on balances", balances);

        // This struct is basically a lambda on steroids. I'm using it to transform the array of balances into an array
        // of datagrams by looking up relevant datagrams on each balance. Also, make sure the newest balance's decision
        // is in the front (I don't care about preserving order) so that the newest decision will be returned.
        struct {
            // Capture this
            BlockchainWalletApi* wrapper;
            // Capture contestId
            QString contestId;
            // For each balance, look up the datagram containing the relevant decision. Store the promises in this array
            kj::ArrayBuilder<kj::Promise<kj::Maybe<DatagramResponse>>> datagramPromises;

            Reader newestBalance;
            void operator() (Reader balance) {
                if (datagramPromises.size() == 0) {
                    // First balance. Nothing to compare.
                    newestBalance = balance;
                }

                // Start a lookup for the datagram and store the promise.
                auto request = wrapper->m_chain.getDatagramByBalanceRequest();
                request.setBalanceId(balance.getId());
                request.initKey().initKey().initDecisionKey().setBalanceId(balance.getId());
                datagramPromises.add(request.send().then([](auto response) -> kj::Maybe<DatagramResponse> {
                        return kj::mv(response);
                    }, [](kj::Exception e) -> kj::Maybe<DatagramResponse> {
                        qInfo() << "Got exception when fetching datagram on balance:" << e.getDescription().cStr();
                        return {};
                    })
                );

                // If this balance is newer than the previous newest, move its promise to the front
                if (balance.getCreationOrder() > newestBalance.getCreationOrder()) {
                    newestBalance = balance;
                    std::swap(datagramPromises.front(), datagramPromises.back());
                }
            }
        } accumulator{this, contestId,
                    kj::heapArrayBuilder<kj::Promise<kj::Maybe<DatagramResponse>>>(balances.size()), {}};

        // Process all of the balances. Fetch the appropriate decision for each.
        qDebug() << "Looking for decisions on" << balances.size() << "balances.";
        accumulator = std::for_each(balances.begin(), balances.end(), kj::mv(accumulator));

        return kj::joinPromises(accumulator.datagramPromises.finish());
    }).then([=](kj::Array<kj::Maybe<DatagramResponse>> datagrams) {
        std::unique_ptr<swv::data::Decision> decision;
        for (auto& datagramMaybe : datagrams) {
            KJ_IF_MAYBE(datagram, datagramMaybe) {
                BlobMessageReader reader(datagram->getDatagram().getContent());
                if (decision)
                    decision->updateFields(reader->getRoot<::Decision>());
                else
                    decision.reset(new data::Decision(reader->getRoot<::Decision>()));
                break;
            }
        }
        KJ_REQUIRE(decision.get() != nullptr,
                   "No decision found on chain for the requested contest and owner",
                   contestId.toStdString(),
                   owner.toStdString());

        // Search for non-matching or missing decisions, which would mean the decision is stale.
        for (auto& maybeReader : datagrams) {
            KJ_IF_MAYBE(reader, maybeReader) {
                BlobMessageReader message(reader->getDatagram().getContent());
                data::Decision otherDecision(message->getRoot<::Decision>());
                if (otherDecision != *decision) {
                    emit contestActionRequired(contestId);
                    break;
                }
            } else {
                emit contestActionRequired(contestId);
                break;
            }
        }

        QQmlEngine::setObjectOwnership(decision.get(), QQmlEngine::JavaScriptOwnership);
        return decision.release();
    });

    return kj::mv(promise);
}

void BlockchainWalletApi::unlockWallet() {
    promiseConverter.adopt(m_chain.unlockWalletRequest().send().then([](auto){}));
}

QJSValue BlockchainWalletApi::getBalance(QByteArray id) {
    auto request = m_chain.getBalanceRequest();
    BlobMessageReader reader(convertBlob(id));
    request.setId(reader->getRoot<::BalanceId>());
    return promiseConverter.convert(request.send(), [](auto response) -> QVariant {
        return {QVariant::fromValue<QObject*>(new data::Balance(response.getBalance()))};
    });
}

QJSValue BlockchainWalletApi::getBalancesBelongingTo(QString owner) {
    return promiseConverter.convert(getBalancesBelongingToImpl(owner), [](auto response) -> QVariant {
        auto balances = response.getBalances();
        QList<data::Balance*> results;
        std::transform(balances.begin(), balances.end(), std::back_inserter(results),
                       [](::Balance::Reader r) { return new data::Balance(r); });
        return {QVariant::fromValue(results)};
    });
}

QJSValue BlockchainWalletApi::getContest(QString contestId) {
    // The blockchain wallet API implementation will check that the contest was published by Follow My Vote for us; we
    // don't need to check it again here.
    auto promise = getContestImpl(contestId).then([this, contestId](auto results) {
        auto contest = new data::Contest(contestId, results.getContest().getValue());
        QQmlEngine::setObjectOwnership(contest, QQmlEngine::JavaScriptOwnership);
        auto decision = new data::Decision({}, contest);
        decision->update_contestId(contestId);

        // Defer persistence concerns until later; the contest doesn't know about the QML engine yet so we can't
        // manipulate the QJSValue properties
        QTimer::singleShot(0, [this, contest, decision]() mutable {
            QSettings settings;
            auto key = PERSISTENT_DECISION_KEY.arg(contest->get_id());
            if (settings.contains(key)) {
                try {
                    auto bytes = settings.value(key, QByteArray()).toByteArray();
                    BlobMessageReader reader(convertBlob(bytes));
                    decision->updateFields(reader->getRoot<::Decision>());
                    contest->setPendingDecision(decision);
                } catch (kj::Exception e) {
                    emit this->error(tr("Error when recovering decision: %1")
                                     .arg(QString::fromStdString(e.getDescription())));
                }
            }
            auto persist = [key, decision] {
                QSettings settings;
                capnp::MallocMessageBuilder message;
                auto builder = message.initRoot<::Decision>();

                decision->serialize(builder);
                ReaderPacker packer(builder.asReader());
                settings.setValue(key, convertBlob(packer.array()));
            };
            connect(decision, &data::Decision::opinionsChanged, persist);
            connect(decision, &data::Decision::writeInsChanged, persist);
        });
        contest->setPendingDecision(decision);
        return contest;
    });
    return promiseConverter.convert(kj::mv(promise), [](data::Contest* contest) -> QVariant {
        return {QVariant::fromValue<QObject*>(contest)};
    });
}

QJSValue BlockchainWalletApi::transfer(QString sender, QString recipient, qint64 amount, quint64 coinId, QString memo) {
    auto request = m_chain.transferRequest();
    request.setSendingAccount(sender.toStdString());
    request.setReceivingAccount(recipient.toStdString());
    request.setAmount(amount);
    request.setCoinId(coinId);
    request.setMemo(memo.toStdString());
    return promiseConverter.convert(request.send(), [](capnp::Response<BlockchainWallet::TransferResults> results) {
        return QVariant(QString::fromStdString(results.getTransactionId()));
    });
}

capnp::RemotePromise<BlockchainWallet::GetContestByIdResults> BlockchainWalletApi::getContestImpl(QString contestId) {
    auto request = m_chain.getContestByIdRequest();
    auto id = QByteArray::fromHex(contestId.toLocal8Bit());
    BlobMessageReader reader(convertBlob(id));
    request.setId(reader->getRoot<::ContestId>());
    return request.send();
}

capnp::RemotePromise<BlockchainWallet::GetBalancesBelongingToResults> BlockchainWalletApi::getBalancesBelongingToImpl(QString owner) {
    auto request = m_chain.getBalancesBelongingToRequest();
    request.setOwner(owner.toStdString());
    return request.send();
}

} // namespace swv
