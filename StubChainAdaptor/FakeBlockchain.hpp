#ifndef FAKEBLOCKCHAIN_HPP
#define FAKEBLOCKCHAIN_HPP

#include <blockchainwallet.capnp.h>
#include <backend.capnp.h>
#include <signed.capnp.h>

#include <capnp/message.h>

#include <vector>
#include <map>

namespace swv {

class FakeBlockchain : public BlockchainWallet::Server {
public:
    class BackendStub;
    class ContestCreator;

    FakeBlockchain();
    virtual ~FakeBlockchain();

    ::Backend::Client getBackendStub();

protected:
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

private:
    capnp::MallocMessageBuilder message;
    std::vector<capnp::Orphan<Coin>> coins;
    std::vector<std::string> accounts;
    std::map<uint64_t, capnp::Orphan<Signed<Contest>>> contests;
    std::map<kj::String, std::vector<capnp::Orphan<Balance>>> balances;
    /// Map of Publisher Balance ID to Datagrams owned by that balance
    std::map<std::tuple<uint64_t, uint64_t>, std::vector<capnp::Orphan<::Datagram>>> datagrams;
    uint8_t nextBalanceId = 0;

    // Just call the const version and cast it to non-const
    kj::Maybe<capnp::Orphan<Signed<Contest>>&> getContestById(::ContestId::Reader id) {
        return const_cast<const FakeBlockchain*>(this)->getContestById(id).map([](const auto& c)
                                                                               -> capnp::Orphan<Signed<Contest>>& {
            return const_cast<capnp::Orphan<Signed<Contest>>&>(c);
        });
    }
    kj::Maybe<const capnp::Orphan<Signed<Contest>>&> getContestById(::ContestId::Reader id) const;
    kj::Maybe<capnp::Orphan<Balance>&> getBalanceOrphan(::BalanceId::Reader id);
    kj::Maybe<const capnp::Orphan<Balance>&> getBalanceOrphan(::BalanceId::Reader id) const;
    kj::Maybe<capnp::Orphan<Coin>&> getCoinOrphan(kj::StringPtr name);
    kj::Maybe<const capnp::Orphan<Coin>&> getCoinOrphan(kj::StringPtr name) const;
    ::Signed<::Contest>::Builder createContest();
    ::Balance::Builder createBalance(std::string owner, uint64_t type);
    ::Coin::Builder createCoin();
};

} // namespace swv
#endif // FAKEBLOCKCHAIN_HPP
