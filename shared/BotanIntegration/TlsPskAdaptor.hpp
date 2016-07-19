#ifndef TLSPSKADAPTOR_HPP
#define TLSPSKADAPTOR_HPP

#include <kj/async-io.h>
#include <kj/vector.h>
#include <kj/debug.h>

#include <botan/tls_channel.h>

#include <queue>

namespace fmv {

class TlsPskAdaptor : public kj::AsyncIoStream {
    kj::Own<kj::AsyncIoStream> stream;
    kj::Own<Botan::TLS::Channel> channel;

    /// Promise/fulfiller that resolves when the handshake completes
    /// @{
    // NOTE: The fulfiller must be declared before the promise, as we initialize both in the promise's initialization
    kj::Own<kj::PromiseFulfiller<void>> handshakeCompletedFulfiller;
    kj::ForkedPromise<void> handshakeCompleted = setupHandshakeCompletedPromise();
    kj::ForkedPromise<void> setupHandshakeCompletedPromise();
    /// @}

    /// Promises for data being written to wire
    kj::Vector<kj::Promise<void>> sendPromises;
    /// Promise for more data from the wire
    kj::Promise<void> readPromise = KJ_EXCEPTION(FAILED, "readPromise is not yet set. Call setChannel");

    /// Store incoming app-layer data here until something reads it
    std::queue<kj::byte> incomingApplicationData;
    struct ApplicationDataRequest {
        kj::Own<kj::PromiseFulfiller<size_t>> fulfiller;
        kj::ArrayPtr<kj::byte> buffer;
        size_t minBytes;
        bool throwOnEof;
    };
    std::queue<ApplicationDataRequest> readRequests;
    void processReadRequests();

    void handleEof();
    bool hitEof = false;

    friend class TlsPskAdaptorFactory;
    /// Getters for Botan::TLS::Channel callbacks
    /// @{
    Botan::TLS::Channel::output_fn outputFunction();
    Botan::TLS::Channel::data_cb dataCallback();
    Botan::TLS::Channel::alert_cb alertCallback();
    Botan::TLS::Channel::handshake_cb handshakeCallback();
    /// @}

    /// Begin an asynchronous loop reading bytes from the wire and passing them to the TLS channel
    void startReadLoop();
    /// The body of the read loop
    void processBytes(std::vector<Botan::byte> buffer);

    kj::Promise<void> writeImpl(kj::ArrayPtr<const kj::byte> buffer);

public:
    TlsPskAdaptor(kj::Own<kj::AsyncIoStream> stream);
    virtual ~TlsPskAdaptor(){}

    void setChannel(kj::Own<Botan::TLS::Channel>&& channel) {
        this->channel = kj::mv(channel);
        startReadLoop();
    }

    // AsyncOutputStream interface
    virtual kj::Promise<void> write(const void* data, size_t dataSize) override;
    virtual kj::Promise<void> write(kj::ArrayPtr<const kj::ArrayPtr<const kj::byte>> pieces) override {
        return kj::joinPromises(KJ_MAP(piece, pieces) {
            return write(piece.begin(), piece.size());
        });
    }

    // AsyncInputStream interface
    virtual kj::Promise<size_t> read(void* buffer, size_t minBytes, size_t maxBytes) override;
    virtual kj::Promise<size_t> tryRead(void* buffer, size_t minBytes, size_t maxBytes) override;

    // AsyncIoStream interface
    virtual void shutdownWrite() override;
};

} // nemsapce fmv

#endif // TLSPSKADAPTOR_HPP
