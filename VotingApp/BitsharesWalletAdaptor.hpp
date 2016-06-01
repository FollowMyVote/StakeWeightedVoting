#ifndef BITSHARESWALLETADAPTOR_HPP
#define BITSHARESWALLETADAPTOR_HPP

#include <blockchainwallet.capnp.h>

#include <QtWebSockets>

namespace swv { namespace bts {

/**
 * @brief The BitsharesWalletAdaptor class accepts connections from a Bitshares wallet and creates a BlockchainWallet
 * client out of the connections established
 */
class BitsharesWalletAdaptor : public QWebSocketServer {
    Q_OBJECT

    /// This private class implements the BlockchainWallet::Server that @ref nextWalletClient uses to create its
    /// BlockchainWallet::Client objects
    class BlockchainWalletServer;

public:
    BitsharesWalletAdaptor(const QString& serverName);
    virtual ~BitsharesWalletAdaptor();

    /**
     * @brief Get the nextPendingConnection as a BlockchainWallet::Client
     * @return A BlockchainWallet::Client which wraps the next pending connection
     */
    BlockchainWallet::Client nextWalletClient();
};

} } // namespace swv::bts
#endif // BITSHARESWALLETADAPTOR_HPP
