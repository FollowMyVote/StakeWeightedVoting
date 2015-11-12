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
#include "Promise.hpp"
#include "PromiseWrapper.hpp"

#include "BlockchainAdaptorInterface.hpp"

namespace swv {

const static QString PERSISTENT_DECISION_KEY = QStringLiteral("persistedDecisions/%1");

ChainAdaptorWrapper::ChainAdaptorWrapper(PromiseConverter& promiseWrapper, QObject *parent)
    : QObject(parent),
      promiseConverter(promiseWrapper)
{}

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
        return promiseConverter.wrap(m_adaptor->getCoin(id),
                                   [](::Coin::Reader r) -> QVariantList {
            return {QVariant::fromValue<QObject*>(new Coin(r))};
        });
    return nullptr;
}

Promise* ChainAdaptorWrapper::getCoin(QString symbol)
{
    if (hasAdaptor())
        return promiseConverter.wrap(m_adaptor->getCoin(symbol),
                                   [](::Coin::Reader r) -> QVariantList {
            return {QVariant::fromValue<QObject*>(new Coin(r))};
        });
   return nullptr;
}

Promise* ChainAdaptorWrapper::listAllCoins()
{
    if (hasAdaptor()) {
        return promiseConverter.wrap(m_adaptor->listAllCoins(),
                                   [](kj::Array<::Coin::Reader> coins) -> QVariantList {
            QList<Coin*> results;
            std::transform(coins.begin(), coins.end(), std::back_inserter(results),
                           [](::Coin::Reader r) { return new Coin(r); });
            return {QVariant::fromValue(results)};
        });
    }
    return nullptr;
}

Promise* ChainAdaptorWrapper::getMyAccounts() {
    if (hasAdaptor())
        return promiseConverter.wrap(m_adaptor->getMyAccounts(),
                                   [](kj::Array<QString> accounts) -> QVariantList {
            QStringList results;
            std::transform(accounts.begin(), accounts.end(), std::back_inserter(results),
                           [](QString account) { return account; });
            return {QVariant::fromValue(results)};
        });
    return nullptr;
}

Promise* ChainAdaptorWrapper::getBalance(QByteArray id)
{
    if (hasAdaptor())
        return promiseConverter.wrap(m_adaptor->getBalance(id), [](::Balance::Reader r) -> QVariantList {
            return {QVariant::fromValue<QObject*>(new Balance(r))};
        });
    return nullptr;
}

Promise* ChainAdaptorWrapper::getAccountBalances(QString account)
{
    if (hasAdaptor()) {
        return promiseConverter.wrap(m_adaptor->getBalancesForOwner(account),
                                   [](kj::Array<::Balance::Reader> balances) -> QVariantList {
            QList<Balance*> results;
            std::transform(balances.begin(), balances.end(), std::back_inserter(results),
                           [](::Balance::Reader r) { return new Balance(r); });
            return {QVariant::fromValue(results)};
        });
    }
    return nullptr;
}

Promise* ChainAdaptorWrapper::getBalancesForOwner(QString owner)
{
    QList<Balance*> results;
    if (hasAdaptor()) {
        return promiseConverter.wrap(m_adaptor->getBalancesForOwner(owner),
                                   [](kj::Array<::Balance::Reader> balances) -> QVariantList {
            QList<Balance*> results;
            std::transform(balances.begin(), balances.end(), std::back_inserter(results),
                           [](::Balance::Reader r) { return new Balance(r); });
            return {QVariant::fromValue(results)};
        });
    }
    return nullptr;
}

Promise* ChainAdaptorWrapper::getContest(QString contestId)
{
    QByteArray realContestId = QByteArray::fromHex(contestId.toLocal8Bit());
    if (hasAdaptor())
        return promiseConverter.wrap(m_adaptor->getContest(realContestId),
                                   [this](::Contest::Reader r) -> QVariantList {
            //TODO: Check signature
            auto contest = new Contest(r.getContest(), this);
            auto decision = new OwningWrapper<Decision>(contest);

            // Defer persistence concerns until later; the contest doesn't know about the QML engine yet so we can't
            // manipulate the QJSValue properties
            QTimer::singleShot(0, [this, contest, decision]() mutable {
                QSettings settings;
                auto key = PERSISTENT_DECISION_KEY.arg(contest->id());
                if (settings.contains(key)) {
                    try {
                        auto bytes = settings.value(key, QByteArray()).toByteArray();
                        decision = OwningWrapper<Decision>::deserialize(bytes, contest);
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
                connect(decision, &OwningWrapper<Decision>::opinionsChanged, persist);
                connect(decision, &OwningWrapper<Decision>::writeInsChanged, persist);
            });
            contest->setCurrentDecision(decision);

            return {QVariant::fromValue<QObject*>(contest)};
        });
    return nullptr;
}

Datagram* ChainAdaptorWrapper::getDatagram()
{
    if (hasAdaptor())
        return new Datagram(m_adaptor->createDatagram(), this);
    return nullptr;
}

void ChainAdaptorWrapper::publishDatagram(QByteArray payerBalanceId)
{
    if (hasAdaptor())
        m_adaptor->publishDatagram(payerBalanceId);
}

Promise* ChainAdaptorWrapper::getDatagram(QByteArray balanceId, QString schema)
{
    if (hasAdaptor()) {
        return promiseConverter.wrap(m_adaptor->getDatagram(balanceId, schema),
                                   [this](::Datagram::Reader r) -> QVariantList {
            return {QVariant::fromValue<QObject*>(new OwningWrapper<Datagram>(r, this))};
        });
    }
    return nullptr;
}

} // namespace swv
