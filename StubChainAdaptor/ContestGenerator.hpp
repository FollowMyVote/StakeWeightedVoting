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
#ifndef CONTESTGENERATOR_HPP
#define CONTESTGENERATOR_HPP

#include "contestgenerator.capnp.h"
#include "contest.capnp.h"

#include <vector>

namespace swv {

class ContestGenerator : public ::ContestGenerator::Server
{
public:
    /// Contests will be generated in reverse order of contests; i.e. the vector will be traversed back to front
    /// Thus contests should contain the oldest/least relevant contest at the beginning and the newest at the end
    ContestGenerator(std::vector<Contest::Reader> contests);
    virtual ~ContestGenerator();

protected:
    // ContestGenerator::Server interface
    ::kj::Promise<void> getContest(GetContestContext context);
    ::kj::Promise<void> getContests(GetContestsContext context);
    ::kj::Promise<void> logEngagement(LogEngagementContext);

private:
    std::vector<Contest::Reader> contests;
};

} // namespace swv
#endif // CONTESTGENERATOR_HPP
