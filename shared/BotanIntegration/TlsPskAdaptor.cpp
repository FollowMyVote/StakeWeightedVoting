#include "TlsPskAdaptor.hpp"

namespace fmv {
const static int INITIAL_READ_SIZE = 100;

void TlsPskAdaptor::startReadLoop() {
    auto buffer = std::make_unique<std::vector<Botan::byte>>(INITIAL_READ_SIZE);
    auto promise = stream->tryRead(buffer->data(), 1, buffer->size());
    tasks.add(promise.then([this, buffer = kj::mv(buffer)](size_t bytesRead) mutable {
        buffer->resize(bytesRead);
        processBytes(kj::mv(buffer));
    }));
}

void TlsPskAdaptor::processBytes(std::unique_ptr<std::vector<Botan::byte>> buffer) {
    if (buffer->size() == 0)
        return handleEof();
    auto bytesNeeded = channel->received_data(kj::mv(*buffer));

    if (bytesNeeded) {
        buffer->resize(bytesNeeded);
        auto promise = stream->read(buffer->data(), buffer->size(), buffer->size());
        tasks.add(promise.then([this, buffer = kj::mv(buffer)](size_t bytesRead) mutable {
            buffer->resize(bytesRead);
            processBytes(kj::mv(buffer));
        }));
    } else
        startReadLoop();
}

kj::Promise<void> TlsPskAdaptor::writeImpl(kj::ArrayPtr<const kj::byte> buffer) {
        // As per botan docs, a call to send will trigger at least one call to outputFunction before it returns.
        // outputFunction will populate sendPromises with the promises from the underlying AsyncIoStream. Join those
        // promises as our return promise.
        while(sendPromises.size()) sendPromises.removeLast();
        auto tlsException = kj::runCatchingExceptions([this, buffer] {
            channel->send(static_cast<const Botan::byte*>(buffer.begin()), buffer.size());
        });
        KJ_IF_MAYBE(e, tlsException) {
            while(sendPromises.size()) sendPromises.removeLast();
            return kj::mv(*e);
        }

        return kj::joinPromises(sendPromises.releaseAsArray());
}

kj::ForkedPromise<void> TlsPskAdaptor::setupHandshakeCompletedPromise() {
    auto paf = kj::newPromiseAndFulfiller<void>();
    handshakeCompletedFulfiller = kj::mv(paf.fulfiller);
    return paf.promise.fork();
}

void TlsPskAdaptor::processReadRequests() {
    if (!channel->is_active())
        return;

    while (!incomingApplicationData.empty() && !readRequests.empty()) {
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
    KJ_LOG(INFO, "Got EOF. Canceling reads.");
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
        KJ_LOG(INFO, "TLS Alert", alert.type_string(), dataPtr, dataSize, alert.is_fatal(), alert.is_valid());
        if (alert.is_fatal())
            handleEof();
    };
}

Botan::TLS::Channel::handshake_cb TlsPskAdaptor::handshakeCallback() {
    return [this](const Botan::TLS::Session& session) -> bool {
        KJ_LOG(INFO, "TLS handshake finished", session.ciphersuite().to_string());
        handshakeCompletedFulfiller->fulfill();
        return true;
    };
}

TlsPskAdaptor::TlsPskAdaptor(kj::Own<AsyncIoStream> stream)
    : stream(kj::mv(stream)), errorHandler(*this), tasks(errorHandler) {}

kj::Promise<void> TlsPskAdaptor::write(const void* data, size_t dataSize) {
    if (!channel)
        return KJ_EXCEPTION(DISCONNECTED, "Adaptor cannot be used without a channel. Call setChannel first");

    if (handshakeCompletedFulfiller->isWaiting()) {
        // Defer passing this data to the TLS layer until after the handshake completes
        auto buffer = kj::heapArray<const kj::byte>(static_cast<const kj::byte*>(data), dataSize);
        return handshakeCompleted.addBranch().then([this, buffer = kj::mv(buffer)]() mutable -> kj::Promise<void> {
            return writeImpl(buffer);
        });
    } else {
        // Go ahead and send it now, skip making a copy :D
        return writeImpl(kj::ArrayPtr<const kj::byte>(static_cast<const kj::byte*>(data), dataSize));


    }
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

void TlsPskAdaptor::ErrorHandler::taskFailed(kj::Exception&& exception) {
    KJ_LOG(ERROR, "Exception when reading from wire", exception);
    adaptor.handleEof();
}

} // namespace fmv
