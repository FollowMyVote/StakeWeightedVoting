#include "FakeBlockchain.hpp"
#include "BackendStub.hpp"

#include <kj/debug.h>

#include <algorithm>
#include <string>

namespace swv {
FakeBlockchain::FakeBlockchain() {
    auto coin = createCoin();
    coin.setName("BTS");
    coin.setPrecision(5);
    coin.setCreator("committee-account");

    coin = createCoin();
    coin.setName("FMV");
    coin.setPrecision(0);
    coin.setCreator("follow-my-vote");

    coin = createCoin();
    coin.setName("USD");
    coin.setPrecision(2);
    coin.setCreator("committee-account");

    coin = createCoin();
    coin.setName("VOTE");
    coin.setPrecision(4);
    coin.setCreator("follow-my-vote");

    auto balance = createBalance("nathan", 0);
    balance.setAmount(50000000);

    balance = createBalance("nathan", 1);
    balance.setAmount(10);

    balance = createBalance("nathan", 2);
    balance.setAmount(5000);

    balance = createBalance("nathan", 3);
    balance.setAmount(10000000);

    balance = createBalance("dev.nathanhourt.com", 0);
    balance.setAmount(10000000);

    balance = createBalance("dev.nathanhourt.com", 3);
    balance.setAmount(10000000);

    balance = createBalance("adam", 1);
    balance.setAmount(88);

    balance = createBalance("adam", 3);
    balance.setAmount(10000000);

    balance = createBalance("follow-my-vote", 0);
    balance.setAmount(1000000000);

    auto contest = createContest().getValue();
    contest.setCoin(1);
    contest.setName("Lunch poll");
    contest.setDescription("Where should we go for lunch?");
    contest.setStartTime(1442768400000ull);
    auto contestants = contest.initContestants().initEntries(3);
    contestants[0].setKey("Wikiteria");
    contestants[0].setValue("Cafeteria on the CRC campus");
    contestants[1].setKey("Wicked Taco");
    contestants[1].setValue("Restaurant on Prices Fork");
    contestants[2].setKey("Firehouse");
    contestants[2].setValue("Sub Shop on University City Blvd");

    // Total of 10 contests
    createContest().setValue(contest);
    createContest().setValue(contest);
    createContest().setValue(contest);
    createContest().setValue(contest);
    createContest().setValue(contest);
    createContest().setValue(contest);
    createContest().setValue(contest);
    createContest().setValue(contest);
    createContest().setValue(contest);
}

FakeBlockchain::~FakeBlockchain() {}

Backend::Client FakeBlockchain::getBackendStub() {
    return kj::heap<BackendStub>(*this);
}

kj::Promise<void> FakeBlockchain::getCoinById(BlockchainWallet::Server::GetCoinByIdContext context) {
    auto id = context.getParams().getId();
    KJ_REQUIRE(id < coins.size(), "No such coin found", id);
    context.initResults().setCoin(coins.at(id).getReader());
    return kj::READY_NOW;
}

kj::Promise<void> FakeBlockchain::getCoinBySymbol(BlockchainWallet::Server::GetCoinBySymbolContext context) {
    auto& coin = KJ_REQUIRE_NONNULL(getCoinOrphan(context.getParams().getSymbol()),
                                    "No such coin found", context.getParams().getSymbol());
    context.initResults().setCoin(coin.getReader());
    return kj::READY_NOW;
}

kj::Promise<void> FakeBlockchain::getAllCoins(BlockchainWallet::Server::GetAllCoinsContext context) {
    auto results = context.initResults().initCoins(coins.size());
    auto index = 0u;
    for (const auto& coin : coins)
        results.setWithCaveats(index++, coin.getReader());
    return kj::READY_NOW;
}

kj::Promise<void> FakeBlockchain::listMyAccounts(BlockchainWallet::Server::ListMyAccountsContext context) {
    auto results = context.initResults().initAccountNames(2);
    results.set(0, "nathan");
    results.set(1, "dev.nathanhourt.com");
    return kj::READY_NOW;
}

kj::Promise<void> FakeBlockchain::getBalance(BlockchainWallet::Server::GetBalanceContext context) {
    auto& balance = KJ_REQUIRE_NONNULL(getBalanceOrphan(context.getParams().getId()),
                                       "Could not find the specified balance", context.getParams().getId());
    context.initResults().setBalance(balance.getReader());
    return kj::READY_NOW;
}

kj::Promise<void> FakeBlockchain::getBalancesBelongingTo(BlockchainWallet::Server::GetBalancesBelongingToContext context) {
    auto owner = context.getParams().getOwner();
    auto itr = balances.find(kj::heapString(owner));
    KJ_REQUIRE(itr != balances.end(), "Could not find the specified owner", owner);

    auto& balances = itr->second;
    auto results = context.initResults().initBalances(balances.size());
    auto index = 0u;
    for (const auto& balance : balances)
        // setWithCaveats explained in this page: https://capnproto.org/cxx.html
        results.setWithCaveats(index++, balance.getReader());

    return kj::READY_NOW;
}

kj::Promise<void> FakeBlockchain::getContestById(BlockchainWallet::Server::GetContestByIdContext context) {
    auto& contest = KJ_REQUIRE_NONNULL(getContestById(context.getParams().getId()),
                                      "Could not find the specified contest", context.getParams().getId());
    context.initResults().setContest(contest.getReader());
    return kj::READY_NOW;
}

bool operator== (const ::Datagram::DatagramKey::Reader& a, const ::Datagram::DatagramKey::Reader& b) {
    if (a.getKey().which() != b.getKey().which())
        return false;
    if (a.getKey().isContestKey()) {
        auto ac = a.getKey().getContestKey().getCreator();
        auto bc = b.getKey().getContestKey().getCreator();
        if (ac.which() != bc.which())
            return false;
        if (ac.isAnonymous())
            return bc.isAnonymous();
        return ac.getSignature().getId() == bc.getSignature().getId() &&
                ac.getSignature().getSignature() == bc.getSignature().getSignature();
    }
    auto ad = a.getKey().getDecisionKey().getBalanceId();
    auto bd = b.getKey().getDecisionKey().getBalanceId();
    return ad.getAccountInstance() == bd.getAccountInstance() && ad.getCoinInstance() == bd.getCoinInstance();
}
bool operator!= (const ::Datagram::DatagramKey::Reader& a, const ::Datagram::DatagramKey::Reader& b) {
    return !(a==b);
}

kj::Promise<void> FakeBlockchain::getDatagramByBalance(BlockchainWallet::Server::GetDatagramByBalanceContext context) {
    auto key = context.getParams().getKey();
    auto balanceId = context.getParams().getBalanceId();
    auto& datagramList = datagrams[std::make_tuple(balanceId.getAccountInstance(), balanceId.getCoinInstance())];
    auto itr = std::find_if(datagramList.begin(), datagramList.end(), [key](const capnp::Orphan<::Datagram>& datagram) {
        return datagram.getReader().getKey() == key;
    });
    KJ_REQUIRE(itr != datagramList.end(), "No matching datagrams found.", balanceId, key);

    context.initResults().setDatagram(itr->getReader());
    return kj::READY_NOW;
}

kj::Promise<void> FakeBlockchain::publishDatagram(BlockchainWallet::Server::PublishDatagramContext context) {
    auto& payerBalance = KJ_REQUIRE_NONNULL(getBalanceOrphan(context.getParams().getPayingBalance()),
                                           "Could not find the payer balance");
    auto publishingBalance = context.getParams().getPublishingBalance();
    KJ_REQUIRE_NONNULL(getBalanceOrphan(publishingBalance), "Could not find the publisher balance");

    Balance::Builder builder = payerBalance.get();
    // Arbitrary fee of 10 whatever-this-balance-happens-to-be-in
    KJ_REQUIRE(builder.getAmount() >= 10, "The specified balance cannot pay the fee");
    builder.setAmount(builder.getAmount() - 10);

    auto datagram = context.getParams().getDatagram();
    auto key = std::make_tuple(publishingBalance.getAccountInstance(), publishingBalance.getCoinInstance());
    auto& datagramList = datagrams[key];
    auto itr = std::find_if(datagramList.begin(), datagramList.end(),
                            [datagram](const capnp::Orphan<::Datagram>& datagramOrphan) {
        return datagram.getKey() == datagramOrphan.getReader().getKey();
    });
    if (itr == datagramList.end()) {
        KJ_LOG(DBG, "Storing datagram in stub chain", context.getParams().getPublishingBalance());
        datagramList.emplace_back(message.getOrphanage().newOrphanCopy(datagram));
    } else {
        KJ_LOG(DBG, "Updating datagram in stub chain", context.getParams().getPublishingBalance());
        *itr = message.getOrphanage().newOrphanCopy(datagram);
    }
    return kj::READY_NOW;
}

kj::Promise<void> FakeBlockchain::transfer(BlockchainWallet::Server::TransferContext context) {
    auto sender = context.getParams().getSendingAccount();
    auto recipient = context.getParams().getReceivingAccount();
    auto amount = context.getParams().getAmount();
    auto coinId = context.getParams().getCoinId();
    KJ_LOG(DBG, "Attempting to transfer", sender, recipient, amount, coinId);

    auto senderBalances = balances.find(kj::heapString(sender));
    KJ_REQUIRE(senderBalances != balances.end(), "Cannot transfer because sender has no balances", sender);

    int64_t senderFunds = 0;
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

    auto newBalance = createBalance(recipient, coinId);
    newBalance.setAmount(amountRemaining);

    return kj::READY_NOW;
}

inline bool operator==(const ContestId::Reader& a, const ContestId::Reader& b) {
    return a.getOperationId() == b.getOperationId();
}
inline bool operator==(const BalanceId::Reader& a, const BalanceId::Reader& b) {
    return a.getAccountInstance() == b.getAccountInstance() && a.getCoinInstance() == b.getCoinInstance();
}

kj::Maybe<const capnp::Orphan<Signed<Contest>>&> FakeBlockchain::getContestById(::ContestId::Reader id) const {
    auto itr = contests.find(id.getOperationId());
    if (itr == contests.end()) return {};
    return itr->second;
}

kj::Maybe<capnp::Orphan<Balance>&> FakeBlockchain::getBalanceOrphan(::BalanceId::Reader id) {
    for (auto& bals : balances) {
        auto itr = std::find_if(bals.second.begin(), bals.second.end(), [id](const capnp::Orphan<Balance>& balance) {
            return balance.getReader().getId() == id;
        });
        if (itr != bals.second.end())
            return *itr;
    }
    return {};
}

kj::Maybe<const capnp::Orphan<Balance>&> FakeBlockchain::getBalanceOrphan(::BalanceId::Reader id) const {
    for (const auto& bals : balances) {
        auto itr = std::find_if(bals.second.begin(), bals.second.end(), [id](const capnp::Orphan<Balance>& balance) {
            return balance.getReader().getId() == id;
        });
        if (itr != bals.second.end())
            return *itr;
    }
    return {};
}

kj::Maybe<capnp::Orphan<Coin>&> FakeBlockchain::getCoinOrphan(kj::StringPtr name) {
    auto itr = std::find_if(coins.begin(), coins.end(), [name] (const capnp::Orphan<Coin>& coin) {
        return coin.getReader().getName() == name;
    });
    if (itr == coins.end())
        return {};
    return *itr;
}

kj::Maybe<const capnp::Orphan<Coin>&> FakeBlockchain::getCoinOrphan(kj::StringPtr name) const {
    auto itr = std::find_if(coins.begin(), coins.end(), [name] (const capnp::Orphan<Coin>& coin) {
        return coin.getReader().getName() == name;
    });
    if (itr == coins.end())
        return {};
    return *itr;
}

Signed<Contest>::Builder FakeBlockchain::createContest() {
    auto pair = contests.insert(std::make_pair(contests.size(), message.getOrphanage().newOrphan<Signed<Contest>>()));
    return pair.first->second.get();
}

Balance::Builder FakeBlockchain::createBalance(std::string owner, uint64_t type) {
    auto accountId = accounts.size();
    auto itr = std::find(accounts.begin(), accounts.end(), owner);
    if (itr != accounts.end())
        accountId = itr - accounts.begin();
    else
        accounts.emplace_back(owner);

    balances[kj::heapString(owner)].emplace_back(message.getOrphanage().newOrphan<::Balance>());
    auto newBalance = balances[kj::heapString(owner)].back().get();
    newBalance.getId().setAccountInstance(accountId);
    newBalance.getId().setCoinInstance(type);
    newBalance.setType(type);
    return newBalance;
}

Coin::Builder FakeBlockchain::createCoin() {
    coins.emplace_back(message.getOrphanage().newOrphan<::Coin>());
    auto& newCoin = coins.back();
    newCoin.get().setId(coins.size() - 1);
    return newCoin.get();
}

} // namespace swv
