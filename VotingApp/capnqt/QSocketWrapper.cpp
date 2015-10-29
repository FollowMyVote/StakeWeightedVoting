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

QSocketWrapper::~QSocketWrapper() throw() {
    KJ_IF_MAYBE(fulfiller, pendingFulfiller) {
        if (readContext.truncateForEof)
            (*fulfiller)->fulfill(kj::mv(readContext.bytesRead));
        else
            (*fulfiller)->reject(KJ_EXCEPTION(DISCONNECTED, "Socket was disconnected before read completed."));
    }

    if (stream.state() != QAbstractSocket::UnconnectedState && stream.state() != QAbstractSocket::ClosingState)
        stream.close();
}

kj::Promise<void> QSocketWrapper::write(const void* buffer, size_t size) {
    stream.write((const char*)buffer, size);
    return kj::READY_NOW;
}

kj::Promise<void> QSocketWrapper::write(kj::ArrayPtr<const kj::ArrayPtr<const kj::byte> > pieces) {
    for (const kj::ArrayPtr<const kj::byte> piece : pieces)
        stream.write((const char*)piece.begin(), piece.size());
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
    // Several possibilities here:
    // - maxBytes are available now, so we return all of them immediately.
    // - Less than maxBytes are available now, but more than minBytes, so we return all of them immediately.
    // - Less than minBytes are available, so we construct a promise and wait for more data

    // In the first two cases, we can read immediately, so attemptRead will succeed.
    auto bytesRead = attemptRead((char*)buffer, minBytes, maxBytes, truncateForEof);
    if (bytesRead)
        return bytesRead;
    // No point in making a promise if we already know we'll have to break it.
    if (atEof())
        return KJ_EXCEPTION(DISCONNECTED, "Stream disconnected with less than minBytes readable.", minBytes);

    // Third option: make a promise
    auto paf = kj::newPromiseAndFulfiller<size_t>();
    pendingFulfiller = kj::mv(paf.fulfiller);
    readContext.bytesRead = 0;
    readContext.minBytes = minBytes;
    readContext.maxBytes = maxBytes;
    readContext.buffer = (char*)buffer;
    readContext.truncateForEof = truncateForEof;
    QObject::connect(&stream, &QAbstractSocket::readyRead, this, [this] {
        if (readContext.maxBytes > 0) {
            auto bytesRead = bytesIn(readContext.buffer,
                                     readContext.minBytes,
                                     readContext.maxBytes,
                                     readContext.truncateForEof);
            readContext.bytesRead += bytesRead;
            readContext.minBytes -= bytesRead;
            readContext.maxBytes -= bytesRead;
        }
    });

    return kj::mv(paf.promise);
}

size_t QSocketWrapper::attemptRead(char* buffer, size_t minBytes, size_t maxBytes, bool truncateForEof) {
    size_t availableNow = stream.bytesAvailable();
    if (availableNow >= maxBytes) {
        // First option: return all maxBytes now
        stream.read((char*)buffer, maxBytes);
        return maxBytes;
    }
    if (availableNow >= minBytes || (truncateForEof && atEof())) {
        // Second option: return what's available now
        stream.read((char*)buffer, availableNow);
        return availableNow;
    }
    return 0;
}

size_t QSocketWrapper::bytesIn(char* buffer, size_t minBytes, size_t maxBytes, bool truncateForEof) {
    KJ_IF_MAYBE(fulfiller, pendingFulfiller) {
        // If we now have enough bytes, attemptRead succeeds and we're done
        auto bytesRead = attemptRead(buffer, minBytes, maxBytes, truncateForEof);
        if (bytesRead) {
            auto copy = bytesRead;
            (*fulfiller)->fulfill(kj::mv(copy));
            fulfiller = nullptr;
            return bytesRead;
        }
        if (atEof())
            (*fulfiller)->reject(KJ_EXCEPTION(DISCONNECTED,
                                              "Stream disconnected with less than minBytes readable.",
                                              minBytes));
        // Not enough bytes yet. Keep waiting.
        return 0;
    }

    // There's no request to read... Read nothing.
    // In practice, this probably never happens, but it's still possible.
    return 0;
}
