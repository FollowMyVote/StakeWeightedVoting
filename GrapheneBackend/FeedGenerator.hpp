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
#ifndef FEEDGENERATOR_HPP
#define FEEDGENERATOR_HPP

#include "Contest.hpp"

#include <contestgenerator.capnp.h>

namespace graphene { namespace chain { class database; } }

namespace swv {

class FeedGenerator : public ContestGenerator::Server {
    const Contest* currentContest = nullptr;
    const gch::database& db;
    const ContestObjectMultiIndex::index<ByStartTime>::type& index;

    void populateContest(ContestGenerator::ListedContest::Builder nextContest);

 public:
    FeedGenerator(const Contest* firstContest, const gch::database& db);
    virtual ~FeedGenerator();

protected:
    // ContestGenerator::Server interface
    virtual ::kj::Promise<void> getContest(GetContestContext context) override;
    virtual ::kj::Promise<void> getContests(GetContestsContext context) override;
    virtual ::kj::Promise<void> logEngagement(LogEngagementContext context) override;
};

} // namespace swv

#endif // FEEDGENERATOR_HPP
