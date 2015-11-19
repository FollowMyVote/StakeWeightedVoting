// Copyright (c) 2015 Follow My Vote, Inc.
// Licensed under the MIT License:
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "QSocketWrapper.hpp"

#include <QAbstractSocket>

#include <kj/debug.h>

QSocketWrapper::QSocketWrapper(QAbstractSocket& stream, QObject* parent)
    : QObject(parent), stream(stream)
{
    QObject::connect(&stream, &QAbstractSocket::disconnected, [this]{eof = true;});
}

QSocketWrapper::~QSocketWrapper() noexcept {
    while (!pendingReads.empty()) {
        auto& readContext = pendingReads.front();
        if (readContext.truncateForEof)
            readContext.fulfiller->fulfill(kj::mv(readContext.bytesRead));
        else
            readContext.fulfiller->reject(KJ_EXCEPTION(DISCONNECTED, "Socket was disconnected before read completed."));
        pendingReads.pop();
    }

    if (stream.state() != QAbstractSocket::UnconnectedState && stream.state() != QAbstractSocket::ClosingState)
        stream.close();
}

kj::Promise<void> QSocketWrapper::write(const void* buffer, size_t size) {
    stream.write(static_cast<const char*>(buffer), static_cast<signed>(size));
    return kj::READY_NOW;
}

kj::Promise<void> QSocketWrapper::write(kj::ArrayPtr<const kj::ArrayPtr<const kj::byte> > pieces) {
    for (const kj::ArrayPtr<const kj::byte> piece : pieces)
        stream.write(reinterpret_cast<const char*>(piece.begin()), static_cast<signed>(piece.size()));
    return kj::READY_NOW;
}

kj::Promise<size_t> QSocketWrapper::read(void* buffer, size_t minBytes, size_t maxBytes) {
    return readImpl(buffer, minBytes, maxBytes, false);
}

kj::Promise<size_t> QSocketWrapper::tryRead(void* buffer, size_t minBytes, size_t maxBytes) {
    return readImpl(buffer, minBytes, maxBytes, true);
}

void QSocketWrapper::shutdownWrite() {
    stream.flush();
}

bool QSocketWrapper::atEof() {
    return (eof || stream.state() != QAbstractSocket::ConnectedState);
}

kj::Promise<size_t> QSocketWrapper::readImpl(void* buffer, size_t minBytes, size_t maxBytes, bool truncateForEof) {
    // Three possibilities here:
    // - maxBytes are available now, so we return all of them immediately.
    // - Less than maxBytes are available now, but more than minBytes, so we return all of them immediately.
    // - Less than minBytes are available, so we construct a promise and wait for more data

    // In the first two cases, we can read immediately, so attemptRead will succeed.
    auto bytesRead = attemptRead(static_cast<char*>(buffer), minBytes, maxBytes, truncateForEof);
    if (bytesRead) {
        return bytesRead;
    }
    // No point in making a promise if we already know we'll have to break it.
    if (atEof()) {
        KJ_LOG(ERROR, "Failed read request", buffer, minBytes, stream.bytesAvailable());
        return KJ_EXCEPTION(DISCONNECTED, "Stream disconnected with less than minBytes readable.",
                            minBytes, stream.bytesAvailable());
    }

    // Third option: make a promise
    auto paf = kj::newPromiseAndFulfiller<size_t>();
    pendingReads.emplace(0, minBytes, maxBytes, static_cast<char*>(buffer), kj::mv(paf.fulfiller), truncateForEof);
    QObject::connect(&stream, &QAbstractSocket::readyRead, this, [this] {
        while (!pendingReads.empty() && stream.bytesAvailable()) {
            if (fulfillReadRequest(pendingReads.front()))
                // The current read context was satisfied.
                pendingReads.pop();
            else
                // Not enough bytes to satisfy the next context are available. Wait for more bytes before continuing.
                break;
        }
    });

    return kj::mv(paf.promise);
}

size_t QSocketWrapper::attemptRead(char* buffer, size_t minBytes, size_t maxBytes, bool truncateForEof) {
    size_t availableNow = static_cast<unsigned>(stream.bytesAvailable());
    if (availableNow >= maxBytes) {
        // First option: return all maxBytes now
        stream.read(static_cast<char*>(buffer), static_cast<signed>(maxBytes));
        return maxBytes;
    }
    if (availableNow >= minBytes || (truncateForEof && atEof())) {
        // Second option: return what's available now
        stream.read(static_cast<char*>(buffer), static_cast<signed>(availableNow));
        return availableNow;
    }
    return 0;
}

bool QSocketWrapper::fulfillReadRequest(ReadContext& context) {
    KJ_REQUIRE(context.maxBytes > 0, "Asked to fill buffer in an empty context. Unable to fulfill request.");

    // If we now have enough bytes, attemptRead succeeds and we're done
    if (auto bytesRead = attemptRead(context.buffer, context.minBytes, context.maxBytes, context.truncateForEof)) {
        context.fulfiller->fulfill(kj::mv(bytesRead));
        context.bytesRead += bytesRead;
        context.maxBytes -= bytesRead;
        context.minBytes = 0;
        return true;
    }
    if (atEof()) {
        KJ_LOG(ERROR, "Failed read request", static_cast<void*>(context.buffer), context.minBytes,
               context.maxBytes, context.truncateForEof, stream.bytesAvailable());
        context.fulfiller->reject(KJ_EXCEPTION(DISCONNECTED,
                                               "Stream disconnected with less than minBytes readable.",
                                               stream.bytesAvailable(),
                                               context.minBytes));
        return true;
    }
    // Not enough bytes yet. Keep waiting.
    return false;
}

QSocketWrapper::ReadContext::ReadContext(size_t bytesRead, size_t minBytes, size_t maxBytes, char* buffer,
                                         kj::Own<kj::PromiseFulfiller<size_t> > fulfiller, bool truncateForEof)
    : bytesRead(bytesRead),
      minBytes(minBytes),
      maxBytes(maxBytes),
      buffer(buffer),
      fulfiller(kj::mv(fulfiller)),
      truncateForEof(truncateForEof)
{}
