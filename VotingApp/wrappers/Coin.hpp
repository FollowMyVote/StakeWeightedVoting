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

#ifndef COIN_HPP
#define COIN_HPP

#include "coin.capnp.h"

#include <QObject>

namespace swv {

/**
 * @brief The Coin class is a read-only wrapper for the Coin type
 */
class Coin : public QObject, public ::Coin::Reader
{
    Q_OBJECT
    Q_PROPERTY(quint64 id READ getId CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(qint32 precision READ getPrecision CONSTANT)
public:
    Coin(::Coin::Reader r, QObject* parent = nullptr);

    QString name() const {
        return QString::fromStdString(getName());
    }
};

} // namespace swv

#endif // COIN_HPP
