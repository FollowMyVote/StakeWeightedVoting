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
#include "BackendStub.hpp"
#include "ContestGenerator.hpp"
#include "ContestResults.hpp"

#include "decision.capnp.h"

#include <capnp/serialize.h>

namespace swv {

StubChainAdaptor::BackendStub::BackendStub(StubChainAdaptor &adaptor)
    : adaptor(adaptor)
{}

StubChainAdaptor::BackendStub::~BackendStub()
{}

::kj::Promise<void> StubChainAdaptor::BackendStub::getContestFeed(Backend::Server::GetContestFeedContext context) {
    std::vector<Contest::Reader> feedContests;
    feedContests.reserve(adaptor.contests.size());

    for (const auto& contest : adaptor.contests)
        feedContests.emplace_back(contest.getReader());

    context.initResults().setGenerator(kj::heap<swv::ContestGenerator>(kj::mv(feedContests)));
    return kj::READY_NOW;
}

::kj::Promise<void> StubChainAdaptor::BackendStub::searchContests(Backend::Server::SearchContestsContext context) {
    std::vector<Contest::Reader> feedContests;
    feedContests.reserve(adaptor.contests.size());

    //TODO: implement filtering
    for (const auto& contest : adaptor.contests)
        feedContests.emplace_back(contest.getReader());

    context.initResults().setGenerator(kj::heap<swv::ContestGenerator>(kj::mv(feedContests)));
    return kj::READY_NOW;
}

::kj::Promise<void> StubChainAdaptor::BackendStub::getContestResults(Backend::Server::GetContestResultsContext context) {
    auto contestId = context.getParams().getContestId();
    auto contest = adaptor.getContest(contestId).getContest();
    std::map<int32_t, int64_t> contestantTallies;
    std::map<kj::String, int64_t> writeInTallies;

    for (const auto& datagramPair : adaptor.datagrams) {
        auto datagram = datagramPair.second.getReader();
        // If this is a decision on the contest we're tallying...
        if (datagram.getIndex().getType() == Datagram::DatagramType::DECISION &&
                datagram.getIndex().getKey() == contestId) {
            // Read the decsision out of the datagram
            kj::ArrayInputStream datagramStream(datagram.getContent());
            capnp::InputStreamMessageReader message(datagramStream);
            auto decision = message.getRoot<Decision>();

            if (decision.getContest() != contestId) {
                KJ_LOG(WARNING,
                       "Datagram claiming to be relevant to one contest contains a decision for a different contest",
                       contestId, decision.getContest());
                continue;
            }
            if (decision.getOpinions().size() != 1) {
                KJ_LOG(WARNING, "Decision does not have exactly one opinion. This is currently unsupported",
                       decision);
                continue;
            }

            auto contestant = decision.getOpinions()[0].getContestant();
            if (contestant < 0 ||
                    contestant >= contest.getContestants().getEntries().size() +
                    decision.getWriteIns().getEntries().size()) {
                KJ_LOG(WARNING, "Decision specifies a contestant which does not exist", decision, contest);
                continue;
            }

            KJ_IF_MAYBE(balancePointer, adaptor.getBalanceOrphan(std::get<0>(datagramPair.first))) {
                auto balance = balancePointer->getReader();

                if (balance.getType() != contest.getCoin()) {
                    KJ_LOG(WARNING, "Decision is published on balance which has a different coin than contest",
                           decision, balance);
                    continue;
                }
                if (contestant < contest.getContestants().getEntries().size())
                    contestantTallies[contestant] += balance.getAmount();
                else {
                    contestant -= contest.getContestants().getEntries().size();
                    auto contestantName = kj::heapString(decision.getWriteIns().getEntries()[contestant].getKey());
                    writeInTallies[kj::mv(contestantName)] += balance.getAmount();
                }
            } else {
                KJ_LOG(WARNING, "Unable to find balance for decision",
                       decision, std::get<0>(datagramPair.first).toHex().toStdString());
                continue;
            }
        }
    }

    context.initResults().setResults(kj::heap<ContestResults>(kj::mv(contestantTallies), kj::mv(writeInTallies)));
    return kj::READY_NOW;
}

::kj::Promise<void> StubChainAdaptor::BackendStub::createContest(Backend::Server::CreateContestContext context) {

}

::kj::Promise<void> StubChainAdaptor::BackendStub::getCoinDetails(Backend::Server::GetCoinDetailsContext context) {

}

}
