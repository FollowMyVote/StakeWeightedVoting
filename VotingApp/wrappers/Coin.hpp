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

#include "vendor/QQmlVarPropertyHelpers.h"

#include "coin.capnp.h"

#include <QObject>

namespace swv {

/*!
 * \qmltype Coin
 * \instantiates CoinWrapper
 *
 * The Coin type provides information about a specific type of asset on the blockchain.
 */
class CoinWrapper : public QObject, public ::Coin::Reader
{
    Q_OBJECT
    /*!
     * \qmlproperty int Coin::coinId
     * The ID of the coin
     */
    Q_PROPERTY(quint64 coinId READ getId CONSTANT)
    /*!
     * \qmlproperty string Coin::name
     * The name of the coin
     */
    Q_PROPERTY(QString name READ name CONSTANT)
    /*!
     * \qmlproperty int Coin::precision
     * The maximum number of digits after the decimal point in balances of the coin
     */
    Q_PROPERTY(qint32 precision READ getPrecision CONSTANT)
    /*!
     * \qmlproperty string Coin::creator
     * The name of the account which created the coin
     */
    Q_PROPERTY(QString creator READ creator CONSTANT)
    /*!
     * \qmlproperty int Coin::contestCount
     * The number of currently active contests weighted in the coin
     */
    QML_READONLY_VAR_PROPERTY(qint32, contestCount)
public:
    CoinWrapper(::Coin::Reader r, QObject* parent = nullptr);

    QString name() const {
        return QString::fromStdString(getName());
    }
    QString creator() const {
        return QString::fromStdString(getCreator());
    }
};

} // namespace swv

#endif // COIN_HPP
