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

#ifndef CONTESTRESULTSSERVER_HPP
#define CONTESTRESULTSSERVER_HPP

#include "Objects/Objects.hpp"

#include <contestgenerator.capnp.h>
#include <purchase.capnp.h>

#include <boost/signals2.hpp>
#include <boost/variant.hpp>

#include <vector>

namespace swv {
class VoteDatabase;

class LiveContestResultsServer : public ContestResults::Server {
    VoteDatabase& vdb;
    gch::operation_history_id_type contestId;
    std::vector<boost::signals2::scoped_connection> subscriptions;
    std::vector<::Notifier<::capnp::List<::ContestResults::TalliedOpinion>>::Client> notifiers;

public:
    LiveContestResultsServer(VoteDatabase& vdb, gch::operation_history_id_type contestId);
    virtual ~LiveContestResultsServer(){}

    int64_t totalVotingStake();
protected:
    // Backend::ContestResults::Server interface
    virtual ::kj::Promise<void> results(ResultsContext context) override;
    virtual ::kj::Promise<void> subscribe(SubscribeContext context) override;
    virtual ::kj::Promise<void> decisions(DecisionsContext context) override;

    const Contest& getContest();

    using Results = std::map<boost::variant<int32_t, std::string>, gch::share_type>;

    Results tallyResults();
    void populateResults(capnp::List<ContestResults::TalliedOpinion>::Builder resultsBuilder, Results results);
};

} // namespace swv
#endif // CONTESTRESULTSSERVER_HPP
