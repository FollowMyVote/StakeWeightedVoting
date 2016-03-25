/*
 * Copyright 2015 Follow My Vote, Inc.
 * This file is part of The Follow My Vote Stake-Weighted Voting Application ("SWV").
 *
 * SWV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SWV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SWV.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FCSOCKETWRAPPER_HPP
#define FCSOCKETWRAPPER_HPP

#include <kj/async-io.h>

#include <fc/io/iostream.hpp>

#include <queue>

namespace swv {

class FcStreamWrapper : public kj::AsyncIoStream
{
    // This class implements the kj::AsyncIoStream interface on an fc::iostream.
    //
    // FC streams use co-operative blocking to implement asynchronous IO operations without using promises. To wrap
    // this in kj's promise-based API, we have the methods schedule a read to be handled by an asynchronous FC context
    // and immediately return a promise. The asynchronous contexts will block on the reads and writes, and fulfill the
    // promises as they finish each task.
    //
    // This class assumes that the current thread has a functional FC event loop. If there is no FC event loop, or if
    // that loop is starved, this class will not work.
    //
    // This class allows subsequent calls to read(), tryRead(), or write() before previous calls' promises have
    // resolved, and will fulfill promises in order and with uncorrupted data.

    struct WriteContext;
    struct ReadContext;

    kj::Own<fc::iostream> wrappedStream;

    std::queue<WriteContext> pendingWrites;
    bool writesProcessing = false;
    bool flushWrites = false;

    std::queue<ReadContext> pendingReads;
    bool readsProcessing = false;
    bool eof = false;

public:
    FcStreamWrapper(kj::Own<fc::iostream> wrappedStream);
    virtual ~FcStreamWrapper();

    // AsyncOutputStream interface
    virtual kj::Promise<void> write(const void* buffer, size_t size) override;
    // Schedules a write to occur; returned promise will be fulfilled when the write is finished.
    // Do not delete or modify the buffer until the returned promise resolves.
    // Promise breaks if shutdownWrite() has already been called (meaning there's a bug in caller's code)
    virtual kj::Promise<void> write(kj::ArrayPtr<const kj::ArrayPtr<const kj::byte>> pieces) override;
    // Implemented in terms of the write(buffer, size). The returned promise will be fulfilled when all pieces are
    // written. Do not delete or modify the pieces until the returned promise resolves.
    // Promise breaks if shutdownWrite() has already been called (meaning there's a bug in caller's code)

    // AsyncInputStream interface
    virtual kj::Promise<size_t> read(void* buffer, size_t minBytes, size_t maxBytes) override;
    // Schedules a read of at least minBytes into buffer, and may read up to maxBytes. Promise will be fulfilled as
    // soon as minBytes are available, or broken if EOF occurs before minBytes are read.
    virtual kj::Promise<size_t> tryRead(void* buffer, size_t minBytes, size_t maxBytes) override;
    // Schedules a read of up to maxBytes into buffer. Promise will be fulfilled as soon asn minBytes are available,
    // or EOF occurs. Promise will not be broken by EOF.

    // AsyncIoStream interface
    virtual void shutdownWrite() override;
    // Schedules a flush to occur once all pending writes have finished. Any calls to write() after calling this will
    // return broken promises. This does NOT actually call shutdown on the underlying socket, as fc does not support
    // this, and calling close() will disable reads prematurely.

protected:
    void startWrites();
    void startReads();

    void processWrites();
    void processReads();
};

} // namespace swv

#endif // FCSOCKETWRAPPER_HPP
