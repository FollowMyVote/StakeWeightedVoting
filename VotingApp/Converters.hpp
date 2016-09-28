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

#include <Utilities.hpp>

#include <QByteArray>
#include <QVariantMap>

#include <memory>

namespace swv {

inline QByteArray convertBlob(capnp::Data::Reader data) {
    return QByteArray(reinterpret_cast<const char*>(data.begin()), static_cast<signed>(data.size()));
}
/// Convert a QByteArray into a capnp::Data::Builder.
/// @warning The returned builder just references the QByteArray's memory! Do not change or deallocate the byte array
/// while the returned Builder exists.
inline capnp::Data::Builder convertBlob(QByteArray& data) {
    return capnp::Data::Builder(reinterpret_cast<kj::byte*>(data.data()), data.size());
}
inline QString convertText(capnp::Text::Reader text) {
    return QString::fromStdString(text);
}
inline kj::String convertText(QString source) {
    return kj::heapString(source.toStdString());
}
template <typename T>
inline QList<T> convertList(kj::Array<T>&& kjList) {
    QList<T> result;
    for (const T& elem : kjList)
        result.append(kj::mv(elem));
    return result;
}

namespace _ {
template <typename Struct>
class Storage {
    std::unique_ptr<QByteArray> blob;
    BlobMessageReader message;
    capnp::ReaderFor<Struct> content;

public:
    Storage(QString serialHex)
        : blob(std::make_unique<QByteArray>(QByteArray::fromHex(serialHex.toLocal8Bit()))),
          message(convertBlob(*blob)),
          content(message->getRoot<Struct>()) {}
    Storage(const QByteArray& serial)
        : blob(std::make_unique<QByteArray>(serial)),
          message(convertBlob(*blob)),
          content(message->getRoot<Struct>()) {}

    operator capnp::ReaderFor<Struct>() { return content; }
};
}

/// @return A kj::Own pointer to an opaque object which is implicitly convertible to a Struct::Reader. The reader will
/// be valid for the lifetime of the opaque object.
template <typename Struct>
inline kj::Own<_::Storage<Struct>> convertSerialStruct(QString serialHex) {
    return kj::heap<_::Storage<Struct>>(serialHex);
}
template <typename Struct>
inline kj::Own<_::Storage<Struct>> convertSerialStruct(const QByteArray& serial) {
    return kj::heap<_::Storage<Struct>>(serial);
}

} // namespace swv
