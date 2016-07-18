#include "TlsPskAdaptor.hpp"

namespace fmv {
const static int INITIAL_READ_SIZE = 100;

void TlsPskAdaptor::startReadLoop() {
    auto buffer = std::vector<Botan::byte>(INITIAL_READ_SIZE);
    readPromise = stream->read(buffer.data(), 1, buffer.size()).then(
                      [this, buffer = kj::mv(buffer)](size_t bytesRead) mutable {
        buffer.resize(bytesRead);
        processBytes(kj::mv(buffer));
    }).eagerlyEvaluate(nullptr);
}

void TlsPskAdaptor::processBytes(std::vector<Botan::byte> buffer) {
    if (buffer.size() == 0)
        return handleEof();
    auto bytesNeeded = channel->received_data(kj::mv(buffer));

    if (bytesNeeded) {
        buffer.resize(bytesNeeded);
        readPromise = stream->read(buffer.data(), buffer.size(), buffer.size()).then(
                          [this, buffer = kj::mv(buffer)](size_t bytesRead) mutable {
            buffer.resize(bytesRead);
            processBytes(kj::mv(buffer));
        }).eagerlyEvaluate(nullptr);
    } else
        startReadLoop();
}

void TlsPskAdaptor::processReadRequests() {
    if (!channel->is_active())
        return;

    while (!incomingApplicationData.empty())
        while (!readRequests.empty()) {
            auto& request = readRequests.front();
            if (incomingApplicationData.size() >= request.buffer.size()) {
                for (size_t i = 0; i < request.buffer.size(); ++i) {
                    request.buffer[i] = incomingApplicationData.front();
                    incomingApplicationData.pop();
                }
                request.fulfiller->fulfill(request.buffer.size());
                readRequests.pop();
            } else if (incomingApplicationData.size() >= request.minBytes) {
                auto bytesAvailable = incomingApplicationData.size();
                for (size_t i = 0; i < bytesAvailable; ++i) {
                    request.buffer[i] = incomingApplicationData.front();
                    incomingApplicationData.pop();
                }
                request.fulfiller->fulfill(kj::mv(bytesAvailable));
                readRequests.pop();
            } else
                return;
        }
}

void TlsPskAdaptor::handleEof() {
    KJ_LOG(DBG, "Got EOF. Canceling reads.");
    processReadRequests();
    hitEof = true;
    while (!readRequests.empty()) {
        if (readRequests.front().throwOnEof)
            readRequests.front().fulfiller->reject(KJ_EXCEPTION(DISCONNECTED, "EOF"));
        else
            readRequests.front().fulfiller->fulfill(0);
        readRequests.pop();
    }
}

Botan::TLS::Channel::output_fn TlsPskAdaptor::outputFunction() {
    return [this](const Botan::byte data[], size_t dataSize) {
        auto dataCopy = kj::heapArray<kj::byte>(data, dataSize);
        auto writePromise = stream->write(dataCopy.begin(), dataCopy.size());
        sendPromises.add(writePromise.attach(kj::mv(dataCopy)));
    };
}

Botan::TLS::Channel::data_cb TlsPskAdaptor::dataCallback() {
    return [this](const Botan::byte data[], size_t dataSize) {
        for (size_t i = 0; i < dataSize; ++i)
            incomingApplicationData.push(data[i]);
        processReadRequests();
    };
}

Botan::TLS::Channel::alert_cb TlsPskAdaptor::alertCallback() {
    return [this](Botan::TLS::Alert alert, const Botan::byte data[], size_t dataSize) {
        auto dataPtr = kj::ArrayPtr<const kj::byte>(reinterpret_cast<const kj::byte*>(data), dataSize);
        KJ_LOG(DBG, "TLS Alert", alert.type_string(), dataPtr, dataSize, alert.is_fatal(), alert.is_valid());
        if (alert.is_fatal())
            handleEof();
    };
}

Botan::TLS::Channel::handshake_cb TlsPskAdaptor::handshakeCallback() {
    return [this](const Botan::TLS::Session& session) -> bool {
        KJ_LOG(DBG, "TLS handshake finished", session.ciphersuite().to_string());
        return true;
    };
}

TlsPskAdaptor::TlsPskAdaptor(kj::Own<AsyncIoStream> stream)
    : stream(kj::mv(stream)) {}

kj::Promise<void> TlsPskAdaptor::write(const void* buffer, size_t size) {
    if (!channel)
        return KJ_EXCEPTION(DISCONNECTED, "Adaptor cannot be used without a channel. Call setChannel first");
    // As per botan docs, a call to send will trigger at least one call to outputFunction before it returns.
    // outputFunction will populate sendPromises with the promises from the underlying AsyncIoStream. Join those
    // promises as our return promise.
    while(sendPromises.size()) sendPromises.removeLast();
    auto tlsException = kj::runCatchingExceptions([this, buffer, size] {
        channel->send(static_cast<const Botan::byte*>(buffer), size);
    });
    KJ_IF_MAYBE(e, tlsException) {
        while(sendPromises.size()) sendPromises.removeLast();
        return kj::mv(*e);
    }

    return kj::joinPromises(sendPromises.releaseAsArray());
}

kj::Promise<size_t> TlsPskAdaptor::read(void* buffer, size_t minBytes, size_t maxBytes) {
    if (!channel) {
        KJ_LOG(ERROR, "tryRead with no channel");
        return KJ_EXCEPTION(DISCONNECTED, "Adaptor cannot be used without a channel. Call setChannel first");
    }
    if (hitEof)
        return KJ_EXCEPTION(DISCONNECTED, "EOF");

    auto paf = kj::newPromiseAndFulfiller<size_t>();
    readRequests.emplace(ApplicationDataRequest{kj::mv(paf.fulfiller),
                                                kj::ArrayPtr<kj::byte>(static_cast<kj::byte*>(buffer), maxBytes),
                                                minBytes,
                                                true});
    processReadRequests();
    return kj::mv(paf.promise);
}

kj::Promise<size_t> TlsPskAdaptor::tryRead(void* buffer, size_t minBytes, size_t maxBytes) {
    if (!channel) {
        KJ_LOG(ERROR, "tryRead with no channel");
        return KJ_EXCEPTION(DISCONNECTED, "Adaptor cannot be used without a channel. Call setChannel first");
    }
    if (hitEof)
        return size_t(0);

    auto paf = kj::newPromiseAndFulfiller<size_t>();
    readRequests.emplace(ApplicationDataRequest{kj::mv(paf.fulfiller),
                                                kj::ArrayPtr<kj::byte>(static_cast<kj::byte*>(buffer), maxBytes),
                                                minBytes,
                                                false});
    processReadRequests();
    return kj::mv(paf.promise);

}

void TlsPskAdaptor::shutdownWrite() {
    KJ_REQUIRE(!!channel, "Adaptor cannot be used without a channel. Call setChannel first");
    channel->close();
    stream->shutdownWrite();
}

} // namespace fmv
