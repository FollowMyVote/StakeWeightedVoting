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
#include "StubChainAdaptor.hpp"

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
    coins.emplace_back(kj::mv(coinOrphan));

    coinOrphan = orphanage.newOrphan<Coin>();
    coin = coinOrphan.get();
    coin.setName("FMV");
    coin.setId(1);
    coin.setPrecision(0);
    coins.emplace_back(kj::mv(coinOrphan));

    coinOrphan = orphanage.newOrphan<Coin>();
    coin = coinOrphan.get();
    coin.setName("USD");
    coin.setId(2);
    coin.setPrecision(2);
    coins.emplace_back(kj::mv(coinOrphan));

    std::reference_wrapper<std::vector<capnp::Orphan<Balance>>> bals = balances["nathan"];
    auto balanceOrphan = orphanage.newOrphan<Balance>();
    auto balance = balanceOrphan.get();
    balance.setAmount(50000000);
    balance.initId(1)[0] = 0;
    balance.setType(0);
    bals.get().emplace_back(kj::mv(balanceOrphan));

    balanceOrphan = orphanage.newOrphan<Balance>();
    balance = balanceOrphan.get();
    balance.setAmount(10);
    balance.initId(1)[0] = 1;
    balance.setType(1);
    bals.get().emplace_back(kj::mv(balanceOrphan));

    balanceOrphan = orphanage.newOrphan<Balance>();
    balance = balanceOrphan.get();
    balance.setAmount(5000);
    balance.initId(1)[0] = 2;
    balance.setType(2);
    bals.get().emplace_back(kj::mv(balanceOrphan));

    bals = balances["dev.nathanhourt.com"];

    balanceOrphan = orphanage.newOrphan<Balance>();
    balance = balanceOrphan.get();
    balance.setAmount(10000000);
    balance.initId(1)[0] = 3;
    balance.setType(0);
    bals.get().emplace_back(kj::mv(balanceOrphan));

    bals = balances["adam"];

    balanceOrphan = orphanage.newOrphan<Balance>();
    balance = balanceOrphan.get();
    balance.setAmount(88);
    balance.initId(1)[0] = 4;
    balance.setType(1);
    bals.get().emplace_back(kj::mv(balanceOrphan));

    auto contestOrphan = orphanage.newOrphan<Contest>();
    auto contest = contestOrphan.get();
    auto ucontest = contest.getContest();
    contest.initSignature(0);
    ucontest.initId(1)[0] = 0;
    ucontest.setCoin(1);
    ucontest.setName("Lunch poll");
    ucontest.setDescription("Where should we go for lunch?");
    ucontest.setStartTime(static_cast<uint64_t>(QDateTime::fromString("2015-09-20T12:00:00",
                                                                      Qt::ISODate).toMSecsSinceEpoch()));
    auto tags = ucontest.initTags(1);
    tags[0].setKey("category");
    tags[0].setValue("food");
    auto contestants = ucontest.initContestants(3);
    contestants[0].setName("Wikiteria");
    contestants[0].setDescription("Cafeteria on the CRC campus");
    contestants[1].setName("Wicked Taco");
    contestants[1].setDescription("Restaurant on Prices Fork");
    contestants[2].setName("Firehouse");
    contestants[2].setDescription("Sub Shop on University City Blvd");
    contests.emplace_back(kj::mv(contestOrphan));

    contestOrphan = orphanage.newOrphan<Contest>();
    contest = contestOrphan.get();
    ucontest = contest.getContest();
    contest.initSignature(0);
    ucontest.initId(1)[0] = 1;
    ucontest.setCoin(0);
    ucontest.setName("Upgrade Authorization");
    ucontest.setDescription("Do the BitShares stakeholders accept the upgrade to version 2.0, "
                            "using the Graphene Toolkit?");
    ucontest.setStartTime(static_cast<uint64_t>(QDateTime::fromString("2015-09-11T12:00:00",
                                                                      Qt::ISODate).toMSecsSinceEpoch()));
    tags = ucontest.initTags(1);
    tags[0].setKey("category");
    tags[0].setValue("hard-forks");
    contestants = ucontest.initContestants(2);
    contestants[0].setName("Yes");
    contestants[0].setDescription("Accept the upgrade, and hard-fork to BitShares 2.0");
    contestants[1].setName("No");
    contestants[1].setDescription("Reject the upgrade, and continue using BitShares 0.9.x");
    contests.emplace_back(kj::mv(contestOrphan));

    // Total of 10 contests
    contests.emplace_back(orphanage.newOrphanCopy(contests.back().getReader()));
    contests.back().get().getContest().getId()[0] = 2;
    contests.emplace_back(orphanage.newOrphanCopy(contests.back().getReader()));
    contests.back().get().getContest().getId()[0] = 3;
    contests.emplace_back(orphanage.newOrphanCopy(contests.back().getReader()));
    contests.back().get().getContest().getId()[0] = 4;
    contests.emplace_back(orphanage.newOrphanCopy(contests.back().getReader()));
    contests.back().get().getContest().getId()[0] = 5;
    contests.emplace_back(orphanage.newOrphanCopy(contests.back().getReader()));
    contests.back().get().getContest().getId()[0] = 6;
    contests.emplace_back(orphanage.newOrphanCopy(contests.back().getReader()));
    contests.back().get().getContest().getId()[0] = 7;
    contests.emplace_back(orphanage.newOrphanCopy(contests.back().getReader()));
    contests.back().get().getContest().getId()[0] = 8;
    contests.emplace_back(orphanage.newOrphanCopy(contests.back().getReader()));
    contests.back().get().getContest().getId()[0] = 9;
}

StubChainAdaptor::~StubChainAdaptor() noexcept {}

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
    if (contestId.size() != 1 || char(contestId[0]) < 0 || char(contestId[0]) > 10)
        return KJ_EXCEPTION(FAILED, "Could not find the specified contest.", contestId.toHex().toStdString());
    return contests[static_cast<size_t>(contestId[0])].getReader();
}

kj::Promise<void> StubChainAdaptor::publishDatagram(QByteArray payerBalance)
{
    capnp::Orphan<Datagram> dgram = kj::mv(KJ_REQUIRE_NONNULL(pendingDatagram,
                                                              "No datagram exists to be published. "
                                                              "Call createDatagram first!"));
    Datagram::Reader reader = dgram.getReader();
    auto balanceM = getBalanceOrphan(payerBalance);
    KJ_IF_MAYBE(balance, balanceM) {
        Balance::Builder builder = balance->get();
        KJ_REQUIRE(builder.getAmount() >= 10, "The specified balance cannot pay the fee.");
        builder.setAmount(builder.getAmount() - 10);

        auto schema = reader.getSchema();
        auto key = payerBalance.append(QByteArray::fromRawData(reinterpret_cast<const char*>(schema.begin()),
                                                               static_cast<int>(schema.size())));
        datagrams[key] = kj::mv(dgram);
    } else {
        KJ_FAIL_REQUIRE("Could not find the specified balance.");
    }
    KJ_UNREACHABLE;
}

kj::Promise<Datagram::Reader> StubChainAdaptor::getDatagram(QByteArray balanceId, QString schema) const
{
    auto key = balanceId;
    auto itr = datagrams.find(key.append(QByteArray::fromHex(schema.toLocal8Bit())));
    if (itr == datagrams.end())
        return KJ_EXCEPTION(FAILED, "No datagram belonging to the specified balance with the specified schema found.",
                            balanceId.toHex().data(), schema.toStdString());
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

} // namespace swv
