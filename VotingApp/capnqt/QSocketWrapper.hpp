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

#ifndef QSOCKETWRAPPER
#define QSOCKETWRAPPER

#include <QObject>

#include <kj/async-io.h>

class QAbstractSocket;

class QSocketWrapper : public QObject, public kj::AsyncIoStream
{
    Q_OBJECT
public:
    QSocketWrapper(QAbstractSocket& stream, QObject* parent = nullptr);
    virtual ~QSocketWrapper() throw();

    // AsyncOutputStream interface
    virtual kj::Promise<void> write(const void* buffer, size_t size);
    virtual kj::Promise<void> write(kj::ArrayPtr<const kj::ArrayPtr<const kj::byte> > pieces);

    // AsyncInputStream interface
    // NOTE: After calling read or tryRead, neither function should be called again until the returned promise from the
    // first call has resolved. This limitation could be removed, but that's not trivial, and I don't think it's
    // necessary
    virtual kj::Promise<size_t> read(void* buffer, size_t minBytes, size_t maxBytes);
    virtual kj::Promise<size_t> tryRead(void* buffer, size_t minBytes, size_t maxBytes);

    // AsyncIoStream interface
    virtual void shutdownWrite();

private:
    QAbstractSocket& stream;
    kj::Maybe<kj::Own<kj::PromiseFulfiller<size_t>>> pendingFulfiller;
    bool eof = false;

    struct {
        size_t bytesRead = 0;
        size_t minBytes = 0;
        size_t maxBytes = 0;
        char* buffer;
        bool truncateForEof = false;
    } readContext;

    bool atEof();

    kj::Promise<size_t> readImpl(void* buffer, size_t minBytes, size_t maxBytes, bool truncateForEof);
    size_t attemptRead(char* buffer, size_t minBytes, size_t maxBytes, bool truncateForEof);
    size_t bytesIn(char* buffer, size_t minBytes, size_t maxBytes, bool truncateForEof);
};

#endif // QSOCKETWRAPPER

