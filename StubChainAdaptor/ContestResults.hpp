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
#ifndef CONTESTRESULTS_HPP
#define CONTESTRESULTS_HPP

#include <capnp/backend.capnp.h>

#include <map>

namespace swv {

class ContestResults : public ::Backend::ContestResults::Server
{
public:
    ContestResults(std::map<int32_t, int64_t> contestantTallies, std::map<kj::String, int64_t> writeInTallies);
    virtual ~ContestResults();

    // Backend::ContestResults::Server interface
protected:
    ::kj::Promise<void> results(ResultsContext context);
    ::kj::Promise<void> subscribe(SubscribeContext context);

private:
    std::map<int32_t, int64_t> contestantTallies;
    std::map<kj::String, int64_t> writeInTallies;
};

} // namespace swv
#endif // CONTESTRESULTS_HPP
