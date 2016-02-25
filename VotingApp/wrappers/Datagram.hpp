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

#ifndef DATAGRAM_HPP
#define DATAGRAM_HPP

#include <datagram.capnp.h>

#include <QObject>

namespace swv {

/**
 * @brief The DatagramWrapper class is a read-write wrapper for the Datagram type.
 */
class DatagramWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString schema READ schema WRITE setSchema NOTIFY schemaChanged)
    Q_PROPERTY(QString content READ content WRITE setContent NOTIFY contentChanged)
public:
    using WrappedType = ::Datagram;

    DatagramWrapper(WrappedType::Builder b, QObject* parent = nullptr);
    virtual ~DatagramWrapper() throw() {}

    QString schema() const;
    // Hexadecimal representation of the content of this datagram
    QString content() const;

    /// Retrieve a CapnP Reader for the underlying datagram
    WrappedType::Reader reader() const;
    /// Retrieve a CapnP builder for the underlying datagram. If isConst, this returns a null Maybe.
    WrappedType::Builder builder();

public slots:
    /**
     * @brief Set the schema of the datagram
     * @param schema The new schema
     *
     * If this is a const datagram (isConst is true), this method has no effect.
     */
    void setSchema(QString schema);
    /**
     * @brief Set the content of the datagram
     * @param content The new content
     *
     * If this is a const datagram (isConst is true), this method has no effect.
     *
     * Content is expected to be a hexadecimal-encoded string
     */
    void setContent(QString content);

signals:
    void schemaChanged(QString schema);
    void contentChanged(QString content);

private:
    WrappedType::Builder m_datagram;
};

} // namespace swv

#endif // DATAGRAM_HPP
