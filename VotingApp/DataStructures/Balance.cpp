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

#include "Balance.hpp"
#include "Converters.hpp"

namespace swv { namespace data {

Balance::Balance(::Balance::Reader r, QObject* parent)
    : QObject(parent) {
    updateFields(r);
}

Balance::~Balance() {}

void Balance::updateFields(::Balance::Reader r) {
    m_id = convertBlob(r.getId()).toHex();
    m_amount = r.getAmount();
    m_type = r.getType();
}

} } // namespace swv::data
