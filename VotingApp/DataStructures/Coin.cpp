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

#include "Coin.hpp"
#include "Converters.hpp"

#include <QLocale>
#include <QDebug>

#include <cmath>

namespace swv { namespace data {

Coin::Coin(QObject* parent)
    : QObject(parent)
{}

void Coin::updateFields(::Coin::Reader coin) {
    update_coinId(coin.getId());
    update_creator(convertText(coin.getCreator()));
    update_name(convertText(coin.getName()));
    update_precision(coin.getPrecision());
}

void Coin::updateFields(CoinDetails::Reader details) {
    update_iconUrl(convertText(details.getIconUrl()));
    update_contestCount(details.getActiveContestCount());

    if (details.getVolumeHistory().isHistory()) {
        // TODO: copy history into a QLineSeries property
        // Will do when we integrate Qt Charts into the project
    }
}

QString Coin::formatAmount(qreal amount, bool appendName) {
    // Set the currency symbol to a space and trim it. That's apparently the easiest way to omit the $
    return QLocale::system().toCurrencyString(std::floor(amount)/std::pow(10, m_precision), " ", m_precision).trimmed()
            + (appendName? (" " + m_name) : QString::null);
}

} } // namespace swv::data
