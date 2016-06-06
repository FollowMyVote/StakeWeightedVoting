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
#include "Contest.hpp"

#include <graphene/chain/database.hpp>

namespace swv {

Contest::~Contest() {}

bool Contest::isActive(const gch::database& db) const {
    auto now = db.head_block_time();
    return now >= startTime && now <= endTime;
}

bool Contest::matchesKeyword(std::string keyword) const {
    using std::string;
    auto matchHelper = [&keyword](const std::pair<std::string,std::string>& pair) {
        return pair.first.find(keyword) != string::npos || pair.second.find(keyword) != string::npos;
    };
    return (name.find(keyword) != string::npos ||
            description.find(keyword) != string::npos ||
            std::any_of(contestants.begin(), contestants.end(), matchHelper) ||
            std::any_of(tags.begin(), tags.end(), matchHelper));
}

} // namespace swv
