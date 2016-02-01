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

#include "ChainAdaptorWrapper.hpp"
#include "wrappers/Coin.hpp"
#include "wrappers/Balance.hpp"
#include "wrappers/Datagram.hpp"
#include "wrappers/OwningWrapper.hpp"
#include "wrappers/Converters.hpp"
#include "Promise.hpp"
#include "PromiseConverter.hpp"

#include "BlockchainAdaptorInterface.hpp"

#include <kj/debug.h>

namespace swv {

const static QString PERSISTENT_DECISION_KEY = QStringLiteral("persistedDecisions/%1");
const static QString DECISION_SCHEMA = QStringLiteral("00%1");

ChainAdaptorWrapper::ChainAdaptorWrapper(PromiseConverter& promiseConverter, QObject *parent)
    : QObject(parent),
      promiseConverter(promiseConverter)
{
    connect(this, &ChainAdaptorWrapper::hasAdaptorChanged, this, [this](bool haveAdaptor) {
        if (haveAdaptor)
            this->promiseConverter.adopt(m_adaptor->getMyAccounts().then([this](kj::Array<QString> accounts) {
                                             m_myAccounts = convertList(kj::mv(accounts));
                                             emit myAccountsChanged(m_myAccounts);
                                             qDebug() << m_myAccounts;
                                         }));
    });
}

ChainAdaptorWrapper::~ChainAdaptorWrapper() noexcept
{}

void ChainAdaptorWrapper::setAdaptor(kj::Own<BlockchainAdaptorInterface> adaptor) {
    this->m_adaptor = std::move(adaptor);
    hasAdaptorChanged(hasAdaptor());
}

kj::Own<BlockchainAdaptorInterface> ChainAdaptorWrapper::takeAdaptor() {
    // Do this tmp dance so that when we emit the notify signal, hasAdaptor() will return false.
    auto tmp = std::move(m_adaptor);
    emit hasAdaptorChanged(false);
    return tmp;
}

Promise* ChainAdaptorWrapper::getCoin(quint64 id)
{
    if (hasAdaptor())
        return promiseConverter.convert(m_adaptor->getCoin(id),
                                        [](::Coin::Reader r) -> QVariantList {
            return {QVariant::fromValue<QObject*>(new CoinWrapper(r))};
        });
    return nullptr;
}

Promise* ChainAdaptorWrapper::getCoin(QString symbol)
{
    if (hasAdaptor())
        return promiseConverter.convert(m_adaptor->getCoin(symbol),
                                        [](::Coin::Reader r) -> QVariantList {
            return {QVariant::fromValue<QObject*>(new CoinWrapper(r))};
        });
   return nullptr;
}

Promise* ChainAdaptorWrapper::listAllCoins()
{
    if (hasAdaptor()) {
        return promiseConverter.convert(m_adaptor->listAllCoins(),
                                        [](kj::Array<::Coin::Reader> coins) -> QVariantList {
            QVariantList results;
            std::transform(coins.begin(), coins.end(), std::back_inserter(results),
                           [](::Coin::Reader r) { return QVariant::fromValue((QObject*)new CoinWrapper(r)); });

            return {QVariant::fromValue(results)};
        });
    }

    return nullptr;
}

Promise* ChainAdaptorWrapper::getDecision(QString owner, QString contestId)
{
    auto promise = _getDecision(kj::mv(owner), kj::mv(contestId));
    return promiseConverter.convert(kj::mv(promise), [](OwningWrapper<swv::DecisionWrapper>* d) -> QVariantList {
        return {QVariant::fromValue<QObject*>(d)};
    });
}

kj::Promise<OwningWrapper<DecisionWrapper>*> ChainAdaptorWrapper::_getDecision(QString owner, QString contestId)
{
    if (!hasAdaptor()) return KJ_EXCEPTION(FAILED, "No blockchain adaptor is set.");

    using Reader = ::Balance::Reader;
    auto promise = m_adaptor->getContest(QByteArray::fromHex(contestId.toLocal8Bit())).then([=](::Contest::Reader c) {
        return m_adaptor->getBalancesForOwner(owner).then([c](kj::Array<Reader> balances) {
            return std::make_pair(c, kj::mv(balances));
        });
    }).then([=](std::pair<::Contest::Reader, kj::Array<Reader>> pair) {
        auto contest = kj::mv(pair.first);
        auto balances = kj::mv(pair.second);

        auto newEnd = std::remove_if(balances.begin(), balances.end(), [contest](Reader balance) {
            return balance.getType() != contest.getContest().getCoin();
        });
        KJ_REQUIRE(newEnd - balances.begin() > 0, "No balances found in the contest's coin, so no decision exists.");

        // This struct is basically a lambda on steroids. I'm using it to transform the array of balances into an array
        // of datagrams containing the decision I want on each of those balances. Also, make sure the newest balance's
        // decision is in the front (I don't care about preserving order)
        struct {
            // Capture this
            ChainAdaptorWrapper* wrapper;
            // Capture contestId
            QString contestId;
            // For each balance, look up the datagram containing the relevant decision. Store the promises in this array
            kj::ArrayBuilder<kj::Promise<kj::Maybe<::Datagram::Reader>>> datagramPromises;

            Reader newestBalance;
            void operator() (Reader balance) {
                if (datagramPromises.size() == 0) {
                    // First balance. Nothing to compare.
                    newestBalance = balance;
                }

                // Start a lookup for the datagram and store the promise.
                auto promise = wrapper->m_adaptor->getDatagram(convertBlob(balance.getId()),
                                                               DECISION_SCHEMA.arg(contestId));
                datagramPromises.add(promise.then([](::Datagram::Reader r) -> kj::Maybe<::Datagram::Reader> {
                        return r;
                    }, [](kj::Exception e) -> kj::Maybe<::Datagram::Reader> {
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
                    kj::heapArrayBuilder<kj::Promise<kj::Maybe<::Datagram::Reader>>>(newEnd - balances.begin()), {}};

        // Process all of the balances. Fetch the appropriate decision for each.
        qDebug() << "Looking for decisions on" << newEnd - balances.begin() << "balances.";
        accumulator = std::for_each(balances.begin(), newEnd, kj::mv(accumulator));

        return kj::joinPromises(accumulator.datagramPromises.finish());
    }).then([=](kj::Array<kj::Maybe<::Datagram::Reader>> datagrams) {
        std::unique_ptr<OwningWrapper<swv::DecisionWrapper>> decision;
        for (auto datagramMaybe : datagrams) {
            KJ_IF_MAYBE(datagram, datagramMaybe) {
                decision.reset(OwningWrapper<DecisionWrapper>::deserialize(convertBlob(datagram->getContent())));
                break;
            }
        }
        KJ_REQUIRE(decision.get() != nullptr,
                   "No decision found on chain for the requested contest and owner",
                   contestId.toStdString(),
                   owner.toStdString());

        // Search for non-matching or missing decisions, which would mean the decision is stale.
        for (auto maybeReader : datagrams) {
            KJ_IF_MAYBE(reader, maybeReader) {
                std::unique_ptr<OwningWrapper<swv::DecisionWrapper>> otherDecision(
                            OwningWrapper<swv::DecisionWrapper>::deserialize(convertBlob(reader->getContent())));
                if (*otherDecision != *decision) {
                    emit contestActionRequired(contestId);
                    break;
                }
            } else {
                emit contestActionRequired(contestId);
                break;
            }
        }
        return decision.release();
    });

    return kj::mv(promise);
}

Promise* ChainAdaptorWrapper::getBalance(QByteArray id)
{
    if (hasAdaptor())
        return promiseConverter.convert(m_adaptor->getBalance(id), [](::Balance::Reader r) -> QVariantList {
            return {QVariant::fromValue<QObject*>(new BalanceWrapper(r))};
        });
    return nullptr;
}

Promise* ChainAdaptorWrapper::getAccountBalances(QString account)
{
    if (hasAdaptor()) {
        return promiseConverter.convert(m_adaptor->getBalancesForOwner(account),
                                        [](kj::Array<::Balance::Reader> balances) -> QVariantList {
            QList<BalanceWrapper*> results;
            std::transform(balances.begin(), balances.end(), std::back_inserter(results),
                           [](::Balance::Reader r) { return new BalanceWrapper(r); });
            return {QVariant::fromValue(results)};
        });
    }
    return nullptr;
}

Promise* ChainAdaptorWrapper::getBalancesForOwner(QString owner)
{
    if (hasAdaptor()) {
        return promiseConverter.convert(m_adaptor->getBalancesForOwner(owner),
                                        [](kj::Array<::Balance::Reader> balances) -> QVariantList {
            QList<BalanceWrapper*> results;
            std::transform(balances.begin(), balances.end(), std::back_inserter(results),
                           [](::Balance::Reader r) { return new BalanceWrapper(r); });
            return {QVariant::fromValue(results)};
        });
    }
    return nullptr;
}

Promise* ChainAdaptorWrapper::getContest(QString contestId)
{
    QByteArray realContestId = QByteArray::fromHex(contestId.toLocal8Bit());
    if (hasAdaptor()) {
        auto promise = m_adaptor->getContest(realContestId).then([this](::Contest::Reader r) {
            //TODO: Check signature
            auto contest = new ContestWrapper(r.getContest());
            QQmlEngine::setObjectOwnership(contest, QQmlEngine::JavaScriptOwnership);
            auto decision = new OwningWrapper<DecisionWrapper>(contest);

            // Defer persistence concerns until later; the contest doesn't know about the QML engine yet so we can't
            // manipulate the QJSValue properties
            QTimer::singleShot(0, [this, contest, decision]() mutable {
                QSettings settings;
                auto key = PERSISTENT_DECISION_KEY.arg(contest->id());
                if (settings.contains(key)) {
                    try {
                        auto bytes = settings.value(key, QByteArray()).toByteArray();
                        decision = OwningWrapper<DecisionWrapper>::deserialize(bytes, contest);
                        contest->setCurrentDecision(decision);
                    } catch (kj::Exception e) {
                        emit error(tr("Error when recovering decision: %1")
                                   .arg(QString::fromStdString(e.getDescription())));
                    }
                }
                auto persist = [key, decision] {
                    QSettings settings;
                    auto bytes = decision->serialize();
                    settings.setValue(key, bytes);
                };
                connect(decision, &OwningWrapper<DecisionWrapper>::opinionsChanged, persist);
                connect(decision, &OwningWrapper<DecisionWrapper>::writeInsChanged, persist);
            });
            contest->setCurrentDecision(decision);
            return contest;
        });
        return promiseConverter.convert(kj::mv(promise), [](ContestWrapper* contest) -> QVariantList {
            return {QVariant::fromValue<QObject*>(contest)};
        });
    }
    return nullptr;
}

DatagramWrapper* ChainAdaptorWrapper::getDatagram()
{
    if (hasAdaptor())
        return new DatagramWrapper(m_adaptor->createDatagram(), this);
    return nullptr;
}

Promise* ChainAdaptorWrapper::publishDatagram(QByteArray payerBalanceId, QByteArray publisherBalanceId)
{
    if (hasAdaptor())
        return promiseConverter.convert(m_adaptor->publishDatagram(payerBalanceId, publisherBalanceId));
    return nullptr;
}

Promise* ChainAdaptorWrapper::getDatagram(QString balanceId, QString schema)
{
    if (hasAdaptor()) {
        return promiseConverter.convert(m_adaptor->getDatagram(QByteArray::fromHex(balanceId.toLocal8Bit()), schema),
                                        [this](::Datagram::Reader r) -> QVariantList {
            return {QVariant::fromValue<QObject*>(new OwningWrapper<DatagramWrapper>(r, this))};
        });
    }
    return nullptr;
}

} // namespace swv
