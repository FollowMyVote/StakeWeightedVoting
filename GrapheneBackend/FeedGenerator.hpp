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
