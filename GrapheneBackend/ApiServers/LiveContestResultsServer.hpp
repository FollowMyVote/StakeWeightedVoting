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
