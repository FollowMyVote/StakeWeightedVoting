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

#ifndef BALANCE_HPP
#define BALANCE_HPP

#include "balance.capnp.h"

#include <QObject>

namespace swv {

/**
 * @brief The Balance class is a read-only wrapper for the Balance type
 */
class Balance : public QObject, public ::Balance::Reader
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(qint64 amount READ getAmount CONSTANT)
    Q_PROPERTY(quint64 type READ getType CONSTANT)
public:
    Balance(::Balance::Reader r, QObject* parent = nullptr);

    QString id() const;
};

} // namespace swv

#endif // BALANCE_HPP
