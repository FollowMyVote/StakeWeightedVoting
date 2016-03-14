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
#include "BackendStub.hpp"

#include <capnp/dynamic.h>

#include <QDebug>
#include <QDateTime>

#include <functional>

#include <kj/debug.h>

namespace swv {

StubChainAdaptor::StubChainAdaptor(QObject* parent)
    : QObject(parent)
{
    auto orphanage = message.getOrphanage();
    auto coinOrphan = orphanage.newOrphan<Coin>();
    auto coin = coinOrphan.get();
    coin.setName("BTS");
    coin.setId(0);
    coin.setPrecision(5);
    coin.setCreator("committee-account");
    coins.emplace_back(kj::mv(coinOrphan));

    coinOrphan = orphanage.newOrphan<Coin>();
    coin = coinOrphan.get();
    coin.setName("FMV");
    coin.setId(1);
    coin.setPrecision(0);
    coin.setCreator("follow-my-vote");
    coins.emplace_back(kj::mv(coinOrphan));

    coinOrphan = orphanage.newOrphan<Coin>();
    coin = coinOrphan.get();
    coin.setName("USD");
    coin.setId(2);
    coin.setPrecision(2);
    coin.setCreator("committee-account");
    coins.emplace_back(kj::mv(coinOrphan));

    auto balance = createBalance("nathan");
    balance.setAmount(50000000);
    balance.setType(0);

    balance = createBalance("nathan");
    balance.setAmount(10);
    balance.setType(1);

    balance = createBalance("nathan");
    balance.setAmount(5000);
    balance.setType(2);

    balance = createBalance("dev.nathanhourt.com");
    balance.setAmount(10000000);
    balance.setType(0);

    balance = createBalance("adam");
    balance.setAmount(88);
    balance.setType(1);

    auto contest = createContest().getContest();
    contest.setCoin(1);
    contest.setName("Lunch poll");
    contest.setDescription("Where should we go for lunch?");
    contest.setStartTime(static_cast<uint64_t>(QDateTime::fromString("2015-09-20T12:00:00",
                                                                      Qt::ISODate).toMSecsSinceEpoch()));
    auto contestants = contest.initContestants().initEntries(3);
    contestants[0].setKey("Wikiteria");
    contestants[0].setValue("Cafeteria on the CRC campus");
    contestants[1].setKey("Wicked Taco");
    contestants[1].setValue("Restaurant on Prices Fork");
    contestants[2].setKey("Firehouse");
    contestants[2].setValue("Sub Shop on University City Blvd");

    contest = createContest().getContest();
    contest.setCoin(0);
    contest.setName("Upgrade Authorization");
    contest.setDescription("Do the BitShares stakeholders accept the upgrade to version 2.0, "
                            "using the Graphene Toolkit?");
    contest.setStartTime(static_cast<uint64_t>(QDateTime::fromString("2015-09-11T12:00:00",
                                                                      Qt::ISODate).toMSecsSinceEpoch()));
    contestants = contest.initContestants().initEntries(2);
    contestants[0].setKey("Yes");
    contestants[0].setValue("Accept the upgrade, and hard-fork to BitShares 2.0");
    contestants[1].setKey("No");
    contestants[1].setValue("Reject the upgrade, and continue using BitShares 0.9.x");

    // Total of 10 contests
    createContest().setContest(contests.back().getReader().getContest());
    createContest().setContest(contests.back().getReader().getContest());
    createContest().setContest(contests.back().getReader().getContest());
    createContest().setContest(contests.back().getReader().getContest());
    createContest().setContest(contests.back().getReader().getContest());
    createContest().setContest(contests.back().getReader().getContest());
    createContest().setContest(contests.back().getReader().getContest());
    createContest().setContest(contests.back().getReader().getContest());
}

StubChainAdaptor::~StubChainAdaptor() noexcept {}

::Backend::Client StubChainAdaptor::getBackendStub()
{
    return kj::heap<BackendStub>(*this);
}

kj::Promise<Coin::Reader> StubChainAdaptor::getCoin(quint64 id) const
{
    if (id >= coins.size())
        return KJ_EXCEPTION(FAILED, "Could not find the specified coin.", id);

    return coins[id].getReader();
}

kj::Promise<Coin::Reader> StubChainAdaptor::getCoin(QString symbol) const
{
    auto itr = std::find_if(coins.begin(), coins.end(), [symbol] (const capnp::Orphan<Coin>& coin) {
        return coin.getReader().getName() == symbol.toStdString();
    });
    if (itr == coins.end())
        return KJ_EXCEPTION(FAILED, "Could not find the specified coin.", symbol.toStdString());
    return itr->getReader();
}

kj::Promise<kj::Array<Coin::Reader>> StubChainAdaptor::listAllCoins() const
{
    kj::ArrayBuilder<Coin::Reader> results = kj::heapArrayBuilder<Coin::Reader>(coins.size());
    for (const auto& coin : coins)
        results.add(coin.getReader());
    return results.finish();
}

kj::Promise<kj::Array<QString>> StubChainAdaptor::getMyAccounts() const
{
    return kj::heapArray<QString>({"nathan", "dev.nathanhourt.com"});
}

kj::Promise<Balance::Reader> StubChainAdaptor::getBalance(QByteArray id) const
{
    KJ_IF_MAYBE(balance, getBalanceOrphan(id)) {
        return balance->getReader();
    }
    return KJ_EXCEPTION(FAILED, "Could not find the specified balance.", id.toHex().toStdString());
}

kj::Promise<kj::Array<Balance::Reader>> StubChainAdaptor::getBalancesForOwner(QString owner) const
{
    if (balances.find(owner) == balances.end())
        return KJ_EXCEPTION(FAILED, "Could not find the specified owner.", owner.toStdString());

    auto& bals = balances.at(owner);
    auto results = kj::heapArrayBuilder<Balance::Reader>(bals.size());
    for (const auto& bal : bals)
        results.add(bal.getReader());
    return results.finish();
}

Contest::Reader StubChainAdaptor::getContest(capnp::Data::Reader contestId) const
{
    for (auto& contest : contests)
        if (contest.getReader().getContest().getId() == contestId)
            return contest.getReader();
    KJ_FAIL_REQUIRE("Could not find the specified contest", contestId);
}

Datagram::Builder StubChainAdaptor::createDatagram()
{
    KJ_IF_MAYBE(KJ_UNUSED d, pendingDatagram) {
        KJ_FAIL_REQUIRE("Do not create a second datagram without first publishing the second one");
    }

    pendingDatagram = message.getOrphanage().newOrphan<Datagram>();
    return KJ_ASSERT_NONNULL(pendingDatagram).get();
}

kj::Promise<Contest::Reader> StubChainAdaptor::getContest(QByteArray contestId) const
{
    for (auto& contest : contests)
        if (*contest.getReader().getContest().getId().begin() == *contestId.begin())
            return contest.getReader();
    return KJ_EXCEPTION(FAILED, "Could not find the specified contest", contestId.toHex().toStdString());
}

kj::Promise<void> StubChainAdaptor::publishDatagram(QByteArray payerBalanceId, QByteArray publisherBalanceId)
{
    capnp::Orphan<Datagram> dgram = kj::mv(KJ_REQUIRE_NONNULL(pendingDatagram,
                                                              "No datagram exists to be published. "
                                                              "Call createDatagram first!"));
    pendingDatagram = nullptr;

    auto maybePayerBalance = getBalanceOrphan(payerBalanceId);
    auto maybePublisherBalance = getBalanceOrphan(publisherBalanceId);
    KJ_IF_MAYBE(payerBalance, maybePayerBalance) {
        KJ_IF_MAYBE(publisherBalance, maybePublisherBalance) {
            (void)publisherBalance;
            Balance::Builder builder = payerBalance->get();
            KJ_REQUIRE(builder.getAmount() >= 10, "The specified balance cannot pay the fee");
            builder.setAmount(builder.getAmount() - 10);

            auto index = dgram.getReader().getIndex();
            KJ_LOG(DBG, "Publishing datagram.", publisherBalanceId.toHex().toStdString(), static_cast<uint16_t>(index.getType()), index.getKey());
            std::vector<kj::byte> key(index.getKey().begin(), index.getKey().end());
            datagrams[std::make_tuple(publisherBalanceId, index.getType(), kj::mv(key))] = kj::mv(dgram);
            return kj::READY_NOW;
        } else {
            KJ_FAIL_REQUIRE("Could not find the publisher balance");
        }
    } else {
        KJ_FAIL_REQUIRE("Could not find the payer balance");
    }
    KJ_UNREACHABLE;
}

kj::Promise<void> StubChainAdaptor::transfer(QString sender, QString recipient, qint64 amount, quint64 coinId)
{
    auto senderBalances = balances.find(sender);
    KJ_REQUIRE(senderBalances != balances.end(),
               "Cannot transfer because sender has no balances", sender.toStdString());

    qint64 senderFunds = 0;
    for (const auto& balance : senderBalances->second)
        if (balance.getReader().getType() == coinId)
            senderFunds += balance.getReader().getAmount();
    KJ_REQUIRE(senderFunds >= amount, "Cannot transfer because sender has insufficient funds", senderFunds, amount);

    auto amountRemaining = amount;
    for (auto balance = senderBalances->second.begin(); balance != senderBalances->second.end(); ++balance)
        if (balance->getReader().getType() == coinId) {
            if (balance->getReader().getAmount() <= amountRemaining) {
                amountRemaining -= balance->getReader().getAmount();
                balance = senderBalances->second.erase(balance);
                if (amountRemaining == 0)
                    break;
            } else {
                balance->get().setAmount(balance->get().getAmount() - amountRemaining);
                amountRemaining = 0;
                break;
            }
        }

    auto newBalance = createBalance(recipient);
    newBalance.setType(coinId);
    newBalance.setAmount(amountRemaining);

    return kj::READY_NOW;
}

kj::Promise<Datagram::Reader> StubChainAdaptor::getDatagram(QByteArray balanceId,
                                                            Datagram::DatagramType type,
                                                            QString key) const
{
    auto binaryKey = QByteArray::fromHex(key.toLocal8Bit());
    std::vector<kj::byte> keyVector(binaryKey.begin(), binaryKey.end());
    auto itr = datagrams.find(std::make_tuple(balanceId, type, kj::mv(keyVector)));
    if (itr == datagrams.end())
        return KJ_EXCEPTION(FAILED, "No datagram belonging to the specified balance "
                                    "with the specified type and key found.",
                            balanceId.toHex().data(), static_cast<uint16_t>(type), key.toStdString());
    return itr->second.getReader();
}

kj::Maybe<capnp::Orphan<Balance>&> StubChainAdaptor::getBalanceOrphan(QByteArray id)
{
    for (auto& bals : balances)
    {
        auto itr = std::find_if(bals.second.begin(), bals.second.end(), [id](const capnp::Orphan<Balance>& balance) {
            auto data = balance.getReader().getId();
            QByteArray otherId(reinterpret_cast<const char*>(data.begin()), static_cast<int>(data.size()));
            return otherId == id;
        });
        if (itr != bals.second.end())
            return *itr;
    }
    return {};
}

kj::Maybe<const capnp::Orphan<Balance>&> StubChainAdaptor::getBalanceOrphan(QByteArray id) const
{
    for (const auto& bals : balances)
    {
        auto itr = std::find_if(bals.second.begin(), bals.second.end(), [id](const capnp::Orphan<Balance>& balance) {
            auto data = balance.getReader().getId();
            QByteArray otherId(reinterpret_cast<const char*>(data.begin()), static_cast<int>(data.size()));
            return otherId == id;
        });
        if (itr != bals.second.end())
            return *itr;
    }
    return {};
}

Contest::Builder StubChainAdaptor::createContest()
{
    auto newContest = contests.emplace(contests.begin(), message.getOrphanage().newOrphan<::Contest>())->get();
    newContest.initContest().initId(1)[0] = contests.size() - 1;
    return newContest;
}

Balance::Builder StubChainAdaptor::createBalance(QString owner)
{
    balances[owner].emplace_back(message.getOrphanage().newOrphan<::Balance>());
    auto& newBalance = balances[owner].back();
    newBalance.get().initId(1)[0] = nextBalanceId++;
    return newBalance.get();
}

} // namespace swv
