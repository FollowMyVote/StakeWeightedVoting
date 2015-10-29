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

ChainAdaptorWrapper::ChainAdaptorWrapper(PromiseConverter& promiseWrapper, QObject *parent)
    : QObject(parent),
      promiseWrapper(promiseWrapper)
{}

ChainAdaptorWrapper::~ChainAdaptorWrapper() noexcept
{}

void ChainAdaptorWrapper::setAdaptor(kj::Own<BlockchainAdaptorInterface> adaptor) {
    this->adaptor = std::move(adaptor);
    hasAdaptorChanged(hasAdaptor());
}

kj::Own<BlockchainAdaptorInterface> ChainAdaptorWrapper::getAdaptor() {
    // Do this tmp dance so that when we emit the notify signal, hasAdaptor() will return false.
    auto tmp = std::move(adaptor);
    emit hasAdaptorChanged(false);
    return tmp;
}

Promise* ChainAdaptorWrapper::getCoin(quint64 id)
{
    if (hasAdaptor())
        return promiseWrapper.wrap(adaptor->getCoin(id),
                                   [this](::Coin::Reader r) -> QVariantList {
            return {QVariant::fromValue<QObject*>(new Coin(r, this))};
        });
    return nullptr;
}

Promise* ChainAdaptorWrapper::getCoin(QString symbol)
{
    if (hasAdaptor())
        return promiseWrapper.wrap(adaptor->getCoin(symbol),
                                   [this](::Coin::Reader r) -> QVariantList {
            return {QVariant::fromValue<QObject*>(new Coin(r, this))};
        });
   return nullptr;
}

QList<Coin*> ChainAdaptorWrapper::listAllCoins()
{
    QList<Coin*> results;
    if (hasAdaptor()) {
        auto coins = adaptor->listAllCoins();
        std::transform(coins.begin(), coins.end(), std::back_inserter(results),
                       [this](::Coin::Reader r) { return new Coin(r, this); });
    }
    return results;
}

QStringList ChainAdaptorWrapper::getMyAccounts() {
    if (hasAdaptor())
        return adaptor->getMyAccounts();
    return {};
}

Balance* ChainAdaptorWrapper::getBalance(QByteArray id)
{
    if (hasAdaptor())
        KJ_IF_MAYBE(balance, adaptor->getBalance(id))
            return new Balance(*balance, this);
    return nullptr;
}

QList<Balance*> ChainAdaptorWrapper::getAccountBalances(QString account)
{
    QList<Balance*> results;
    if (hasAdaptor()) {
        auto balances = adaptor->getBalancesForOwner(account);
        std::transform(balances.begin(), balances.end(), std::back_inserter(results),
                       [this](::Balance::Reader r) { return new Balance(r, this); });
    }
    return results;
}

QList<Balance*> ChainAdaptorWrapper::getBalancesForOwner(QString owner)
{
    QList<Balance*> results;
    if (hasAdaptor()) {
        auto balances = adaptor->getBalancesForOwner(owner);
        std::transform(balances.begin(), balances.end(), std::back_inserter(results),
                       [this](::Balance::Reader r) { return new Balance(r, this); });
    }
    return results;
}

Promise* ChainAdaptorWrapper::getContest(QString contestId)
{
    QByteArray realContestId = QByteArray::fromHex(contestId.toLocal8Bit());
    if (hasAdaptor())
        return promiseWrapper.wrap(adaptor->getContest(realContestId),
                                   [this](::Contest::Reader r) -> QVariantList {
            //TODO: Check signature
            return {QVariant::fromValue<QObject*>(new Contest(r.getContest(), this))};
        });
    return nullptr;
}

Datagram* ChainAdaptorWrapper::getDatagram()
{
    if (hasAdaptor())
        return new Datagram(adaptor->createDatagram(), this);
    return nullptr;
}

void ChainAdaptorWrapper::publishDatagram(QByteArray payerBalanceId)
{
    if (hasAdaptor())
        adaptor->publishDatagram(payerBalanceId);
}

Promise* ChainAdaptorWrapper::getDatagram(QByteArray balanceId, QString schema)
{
    if (hasAdaptor()) {
        return promiseWrapper.wrap(adaptor->getDatagram(balanceId, schema),
                                   [this](::Datagram::Reader r) -> QVariantList {
            return {QVariant::fromValue<QObject*>(new OwningWrapper<Datagram>(r, this))};
        });
    }
    return nullptr;
}

} // namespace swv
