#include "FakeBlockchain.hpp"

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

    auto balance = createBalance("nathan");
    balance.setAmount(50000000);
    balance.setType(0);

    balance = createBalance("nathan");
    balance.setAmount(10);
    balance.setType(1);

    balance = createBalance("nathan");
    balance.setAmount(5000);
    balance.setType(2);

    balance = createBalance("nathan");
    balance.setAmount(10000000);
    balance.setType(3);

    balance = createBalance("dev.nathanhourt.com");
    balance.setAmount(10000000);
    balance.setType(0);

    balance = createBalance("dev.nathanhourt.com");
    balance.setAmount(10000000);
    balance.setType(3);

    balance = createBalance("adam");
    balance.setAmount(88);
    balance.setType(1);

    balance = createBalance("adam");
    balance.setAmount(10000000);
    balance.setType(3);

    balance = createBalance("follow-my-vote");
    balance.setAmount(1000000000);
    balance.setType(0);

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

::kj::Promise<void> FakeBlockchain::getCoinById(BlockchainWallet::Server::GetCoinByIdContext context) {
    return kj::READY_NOW;
}

::kj::Promise<void> FakeBlockchain::getCoinBySymbol(BlockchainWallet::Server::GetCoinBySymbolContext context) {
    return kj::READY_NOW;
}

::kj::Promise<void> FakeBlockchain::getAllCoins(BlockchainWallet::Server::GetAllCoinsContext context) {
    return kj::READY_NOW;
}

::kj::Promise<void> FakeBlockchain::listMyAccounts(BlockchainWallet::Server::ListMyAccountsContext context) {
    return kj::READY_NOW;
}

::kj::Promise<void> FakeBlockchain::getBalance(BlockchainWallet::Server::GetBalanceContext context) {
    return kj::READY_NOW;
}

::kj::Promise<void> FakeBlockchain::getBalancesBelongingTo(BlockchainWallet::Server::GetBalancesBelongingToContext context) {
    return kj::READY_NOW;
}

::kj::Promise<void> FakeBlockchain::getContestById(BlockchainWallet::Server::GetContestByIdContext context) {
    return kj::READY_NOW;
}

::kj::Promise<void> FakeBlockchain::getDatagramByBalance(BlockchainWallet::Server::GetDatagramByBalanceContext context) {
    return kj::READY_NOW;
}

::kj::Promise<void> FakeBlockchain::publishDatagram(BlockchainWallet::Server::PublishDatagramContext context) {
    return kj::READY_NOW;
}

::kj::Promise<void> FakeBlockchain::transfer(BlockchainWallet::Server::TransferContext context) {
    return kj::READY_NOW;
}

kj::Maybe<capnp::Orphan<Balance>&> FakeBlockchain::getBalanceOrphan(kj::ArrayPtr<kj::byte> id) {
    for (auto& bals : balances) {
        auto itr = std::find_if(bals.second.begin(), bals.second.end(), [id](const capnp::Orphan<Balance>& balance) {
            return balance.getReader().getId().asBytes() == id;
        });
        if (itr != bals.second.end())
            return *itr;
    }
    return {};
}

kj::Maybe<const capnp::Orphan<Balance>&> FakeBlockchain::getBalanceOrphan(kj::ArrayPtr<kj::byte> id) const {
    for (const auto& bals : balances) {
        auto itr = std::find_if(bals.second.begin(), bals.second.end(), [id](const capnp::Orphan<Balance>& balance) {
            return balance.getReader().getId().asBytes() == id;
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
    auto id = std::vector<kj::byte>(1, contests.size() - 1);
    auto pair = contests.insert(std::make_pair(id, message.getOrphanage().newOrphan<Signed<Contest>>()));
    return pair.first->second.get();
}

Balance::Builder FakeBlockchain::createBalance(kj::StringPtr owner) {
    balances[kj::heapString(owner)].emplace_back(message.getOrphanage().newOrphan<::Balance>());
    auto& newBalance = balances[kj::heapString(owner)].back();
    newBalance.get().initId(1)[0] = nextBalanceId++;
    return newBalance.get();
}

Coin::Builder FakeBlockchain::createCoin() {
    coins.emplace_back(message.getOrphanage().newOrphan<::Coin>());
    auto& newCoin = coins.back();
    newCoin.get().setId(coins.size() - 1);
    return newCoin.get();
}
