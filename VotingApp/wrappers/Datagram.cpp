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

#include "Datagram.hpp"

namespace swv {

DatagramWrapper::DatagramWrapper(::Datagram::Builder b, QObject* parent)
    : QObject(parent),
      m_datagram(b)
{}

QString DatagramWrapper::schema() const
{
    auto result = m_datagram.asReader().getSchema();
    return QByteArray::fromRawData((char*)result.begin(), result.size()).toHex();
}

QString DatagramWrapper::content() const
{
    auto data = m_datagram.asReader().getContent();
    return QByteArray::fromRawData((char*)data.begin(), data.size()).toHex();
}

::Datagram::Reader DatagramWrapper::reader() const
{
    return m_datagram.asReader();
}

::Datagram::Builder DatagramWrapper::builder()
{
    return m_datagram;
}

#define ASSIGN(dataBuilder, hexString, fieldName) \
    QByteArray bin = QByteArray::fromHex(hexString.toLocal8Bit()); \
    auto target = dataBuilder.init ## fieldName(bin.size()); \
    memcpy(target.begin(), bin.data(), target.size())

void DatagramWrapper::setSchema(QString schema)
{
    ASSIGN(m_datagram, schema, Schema);
    emit schemaChanged(schema);
}

void DatagramWrapper::setContent(QString content)
{
    ASSIGN(m_datagram, content, Content);
    emit contentChanged(content);
}

} // namespace swv
