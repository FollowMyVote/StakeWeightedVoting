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
#include "Decision.hpp"

#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <kj/debug.h>

namespace swv {

std::map<int32_t, int32_t> Decision::contestantOpinions(const swv::Contest& contest) const {
    auto predicate = [contestantCount = contest.contestants.size()](std::pair<int32_t, int32_t> opinion) {
        return opinion.first < contestantCount && opinion.first >= 0;
    };

    // Use boost::filter_iterator to return only opinions on contestants (as opposed to write-ins)
    return std::map<int32_t, int32_t>(boost::make_filter_iterator(predicate, opinions.begin(), opinions.end()),
                                      boost::make_filter_iterator(predicate, opinions.end(), opinions.end()));
}

std::map<std::string, int32_t> Decision::writeInOpinions(const Contest& contest) const {
    auto predicate = [contestantCount = contest.contestants.size()](const std::pair<int32_t, int32_t>& opinion) {
        return opinion.first >= contestantCount;
    };
    auto mapping = [this, contestantCount = contest.contestants.size()](const std::pair<int32_t, int32_t>& opinion) {
        KJ_REQUIRE(opinion.first < contestantCount + writeIns.size(), "Opinion references nonexistent candidate!");
        return std::make_pair(writeIns[opinion.first - contestantCount].first, opinion.second);
    };

    // Use boost::filter_iterator to return only opinions on write-ins
    auto filteredBegin = boost::make_filter_iterator(predicate, opinions.begin(), opinions.end());
    auto filteredEnd = boost::make_filter_iterator(predicate, opinions.end(), opinions.end());

    // Use boost::transform_iterator to transform write-in indices into names
    return std::map<std::string, int32_t>(boost::make_transform_iterator(filteredBegin, mapping),
                                          boost::make_transform_iterator(filteredEnd, mapping));
}

} // namespace swv
