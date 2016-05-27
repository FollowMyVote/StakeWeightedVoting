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
#include "ContestCreator.hpp"

#include "decision.capnp.h"

#include <capnp/serialize.h>

#include <kj/debug.h>

#include <chrono>

namespace swv {

capnp::Data::Reader readerize(std::vector<kj::byte>& vector) {
    return kj::ArrayPtr<const kj::byte>(vector.data(), vector.size());
}

FakeBlockchain::BackendStub::BackendStub(FakeBlockchain& chain)
    : chain(chain)
{}

FakeBlockchain::BackendStub::~BackendStub()
{}

::kj::Promise<void> FakeBlockchain::BackendStub::getContestFeed(Backend::Server::GetContestFeedContext context) {
    context.initResults().setGenerator(kj::heap<swv::ContestGenerator>(chain.contests.size()));
    return kj::READY_NOW;
}

::kj::Promise<void> FakeBlockchain::BackendStub::searchContests(Backend::Server::SearchContestsContext context) {
    // TODO: Implement filtering
    context.initResults().setGenerator(kj::heap<swv::ContestGenerator>(chain.contests.size()));
    return kj::READY_NOW;
}

::kj::Promise<void> FakeBlockchain::BackendStub::getContestResults(Backend::Server::GetContestResultsContext context) {
    auto contestId = context.getParams().getContestId();
    auto contest = KJ_REQUIRE_NONNULL(chain.getContestById(contestId),
                                      "Contest not found", contestId).getReader().getValue();
    std::map<int32_t, int64_t> contestantTallies;
    std::map<kj::String, int64_t> writeInTallies;

    for (const auto& datagramPair : chain.datagrams) {
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

            auto balanceId = std::get<0>(datagramPair.first);
            KJ_IF_MAYBE(balancePointer, chain.getBalanceOrphan(readerize(balanceId))) {
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
                       decision, readerize(balanceId));
                continue;
            }
        }
    }

    context.initResults().setResults(kj::heap<ContestResults>(kj::mv(contestantTallies), kj::mv(writeInTallies)));
    return kj::READY_NOW;
}

::kj::Promise<void> FakeBlockchain::BackendStub::createContest(Backend::Server::CreateContestContext context) {
    context.getResults().setCreator(kj::heap<ContestCreator>(chain));
    return kj::READY_NOW;
}

::kj::Promise<void> FakeBlockchain::BackendStub::getCoinDetails(Backend::Server::GetCoinDetailsContext context) {
    auto results = context.getResults().initDetails();
    results.setIconUrl("https://followmyvote.com/wp-content/uploads/2014/02/Follow-My-Vote-Logo.png");
    results.setActiveContestCount(15);

    auto historyLength = context.getParams().getVolumeHistoryLength();
    if (historyLength <= 0)
        results.getVolumeHistory().setNoHistory();
    else {
        auto history = results.getVolumeHistory().initHistory();
        // Get current time, rewound to the most recent hour
        history.setHistoryEndTimestamp(
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()
                    ).count() / (1000 * 60 * 60) * (1000 * 60 * 60));
        // TODO: test that this logic for getting the timestamp is correct
        KJ_LOG(DBG, history.getHistoryEndTimestamp());
        auto histogram = history.initHistogram(historyLength);
        for (int i = 0; i < historyLength; ++i)
            histogram.set(i, 1000000);
    }
    return kj::READY_NOW;
}

}
