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

namespace swv {

CoinWrapper::CoinWrapper(QObject* parent)
    : QObject(parent)
{}

void CoinWrapper::updateFields(Coin::Reader coin) {
    update_coinId(coin.getId());
    update_creator(convertText(coin.getCreator()));
    update_name(convertText(coin.getName()));
    update_precision(coin.getPrecision());
}

void CoinWrapper::updateFields(Backend::CoinDetails::Reader details) {
    update_iconUrl(convertText(details.getIconUrl()));
    update_contestCount(details.getActiveContestCount());
}

} // namespace swv
