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

#pragma once

#include <capnp/message.h>
#include <capnp/serialize-packed.h>

#include <QObject>
#include <QByteArray>
#include <QTimer>

namespace _ {
// Implementation detail of OwningWrapper. This class contains an m_message which cannot be a data member of
// OwningWrapper directly, as it must be constructed prior to calling Wrapper's constructor.
class MessageStorage {
protected:
    virtual ~MessageStorage(){}
    capnp::MallocMessageBuilder m_message;
};
}

/**
 * @brief Decoration on a wrapper type that causes the wrapper to own the underlying message
 *
 * @tparam Wrapper A wrapper type which wraps a capnp data type
 */
template <typename Wrapper>
class OwningWrapper : public _::MessageStorage, public Wrapper {
    capnp::BuilderFor<typename Wrapper::WrappedType> copyData(capnp::ReaderFor<typename Wrapper::WrappedType> r) {
        m_message.setRoot(r);
        return m_message.getRoot<typename Wrapper::WrappedType>();
    }

public:
    OwningWrapper(QObject* parent = nullptr)
        : Wrapper(m_message.initRoot<typename Wrapper::WrappedType>(), parent) {}
    OwningWrapper(capnp::ReaderFor<typename Wrapper::WrappedType> r, QObject* parent = nullptr)
        : Wrapper(copyData(r), parent) {}
    virtual ~OwningWrapper() throw() {}

    QByteArray serialize()
    {
        // Future optimization: only allocate the necessary space
        // Right now we allocate enough space for an unpacked message, then write a packed message and shrink to fit
        // When I figure out how to predict the size of a packed message before writing it, I'll fix this
        QByteArray buffer(capnp::computeSerializedSizeInWords(m_message) * capnp::BYTES_PER_WORD, 0);
        kj::ArrayOutputStream arrayStream(kj::ArrayPtr<kj::byte>((unsigned char*)buffer.data(), buffer.size()));
        capnp::writePackedMessage(arrayStream, m_message);
        buffer.resize(arrayStream.getArray().size());
        return buffer;
    }
};
