#ifndef FAKEBLOCKCHAIN_HPP
#define FAKEBLOCKCHAIN_HPP

#include <blockchainwallet.capnp.h>
#include <backend.capnp.h>
#include <signed.capnp.h>

#include <capnp/message.h>

#include <vector>
#include <map>

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
    std::map<std::vector<kj::byte>, capnp::Orphan<Signed<Contest>>> contests;
    std::map<kj::String, std::vector<capnp::Orphan<Balance>>> balances;
    /// Map of (Publisher Balance ID, Type, Key) to Datagram
    /// See shared/capnp/datagram.capnp for a list of datagram types and what the key is for each type
    std::map<std::tuple<std::vector<kj::byte>, Datagram::DatagramType, std::vector<kj::byte>>,
             capnp::Orphan<::Datagram>> datagrams;
    kj::Maybe<capnp::Orphan<::Datagram>> pendingDatagram;
    uint8_t nextBalanceId = 0;

    kj::Maybe<capnp::Orphan<Balance>&> getBalanceOrphan(kj::ArrayPtr<kj::byte> id);
    kj::Maybe<const capnp::Orphan<Balance>&> getBalanceOrphan(kj::ArrayPtr<kj::byte> id) const;
    kj::Maybe<capnp::Orphan<Coin>&> getCoinOrphan(kj::StringPtr name);
    kj::Maybe<const capnp::Orphan<Coin>&> getCoinOrphan(kj::StringPtr name) const;
    ::Signed<::Contest>::Builder createContest();
    ::Balance::Builder createBalance(kj::StringPtr owner);
    ::Coin::Builder createCoin();
};

#endif // FAKEBLOCKCHAIN_HPP
