#ifndef LIVEDECISIONGENERATOR_HPP
#define LIVEDECISIONGENERATOR_HPP

#include "Objects/Objects.hpp"

#include <decisiongenerator.capnp.h>
#include <generator.capnp.h>

namespace swv {
class VoteDatabase;
using DecisionGenerator = Generator<::DecisionInfo>;

/**
 * @brief The SnapshotDecisionGenerator class generates decisions from a snapshot taken at its instantiation
 *
 * A decision generator returns decisions to the caller a few at a time. A snapshot decision generator takes a snapshot
 * of all of the decisions on a contest when it is instantiated, and returns those decisions to the caller.
 */
class SnapshotDecisionGenerator : public DecisionGenerator::Server {
    const VoteDatabase& vdb;
    std::deque<DecisionObjectId> snapshotDecisionIds;

public:
    SnapshotDecisionGenerator(graphene::chain::operation_history_id_type contestId, const VoteDatabase& vdb);
    virtual ~SnapshotDecisionGenerator(){}

protected:
    virtual kj::Promise<void> getValues(GetValuesContext context);

    // Returns true if snapshotDecisionIds[0] references a decision which was counted; false otherwise
    bool firstDecisionWasCounted();
};

} // namespace swv
#endif // LIVEDECISIONGENERATOR_HPP
