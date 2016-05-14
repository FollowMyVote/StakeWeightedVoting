#ifndef CONTESTRESULTSSERVER_HPP
#define CONTESTRESULTSSERVER_HPP

#include "Types.hpp"

#include <backend.capnp.h>
#include <purchase.capnp.h>

#include <boost/signals2.hpp>

#include <vector>

namespace swv {
class VoteDatabase;

class ContestResultsServer : public Backend::ContestResults::Server
{
    VoteDatabase& vdb;
    gch::operation_history_id_type contestId;
    std::vector<boost::signals2::scoped_connection> subscriptions;
    std::vector<::Notifier<::capnp::List<::Backend::ContestResults::TalliedOpinion>>::Client> notifiers;

public:
    ContestResultsServer(VoteDatabase& vdb, gch::operation_history_id_type contestId);

protected:
    // Backend::ContestResults::Server interface
    virtual ::kj::Promise<void> results(ResultsContext context) override;
    virtual ::kj::Promise<void> subscribe(SubscribeContext context) override;
    const Contest& getContest();
    void populateResults(capnp::List<Backend::ContestResults::TalliedOpinion>::Builder results,
                         const Contest& contest);
};

} // namespace swv
#endif // CONTESTRESULTSSERVER_HPP
