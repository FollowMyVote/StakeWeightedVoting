#ifndef BITSHARESWALLETADAPTOR_HPP
#define BITSHARESWALLETADAPTOR_HPP

#include <blockchainwallet.capnp.h>

#include <QtWebSockets>

namespace swv { namespace bts {

/**
 * @brief The BitsharesWalletBridge class accepts connections from a Bitshares wallet and creates a BlockchainWallet
 * client out of the connections established
 */
class BitsharesWalletBridge : public QWebSocketServer {
    Q_OBJECT

    /// This private class implements the BlockchainWallet::Server that @ref nextWalletClient uses to create its
    /// BlockchainWallet::Client objects
    class BlockchainWalletServer;

    struct ClientContext{
        QMetaObject::Connection connection;
        kj::Own<kj::PromiseFulfiller<BlockchainWallet::Client>> fulfiller;
    };
    std::map<int, ClientContext> contexts;
    int nextContextId = 0;

public:
    BitsharesWalletBridge(const QString& serverName);
    virtual ~BitsharesWalletBridge();

    /**
     * @brief Get the nextPendingConnection as a BlockchainWallet::Client
     * @return A BlockchainWallet::Client which wraps the next pending connection
     */
    kj::Promise<BlockchainWallet::Client> nextWalletClient();
};

} } // namespace swv::bts
#endif // BITSHARESWALLETADAPTOR_HPP
