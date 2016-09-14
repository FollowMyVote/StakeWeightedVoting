#include "SnapshotDecisionGenerator.hpp"
#include "VoteDatabase.hpp"

#include <kj/debug.h>

namespace swv {

SnapshotDecisionGenerator::SnapshotDecisionGenerator(gch::operation_history_id_type contestId, const VoteDatabase& vdb)
    : vdb(vdb) {
    auto& index = vdb.decisionIndex().indices().get<ByContest>();
    auto range = index.equal_range(contestId);
    std::transform(range.first, range.second, std::back_inserter(snapshotDecisionIds), [](const Decision& decision) {
        return decision.id;
    });
    KJ_DBG("Took snapshot of decisions", contestId.instance.value, snapshotDecisionIds.size(), index.size());
}

kj::Promise<void> SnapshotDecisionGenerator::getValues(GetValuesContext context) {
    KJ_DBG(__FUNCTION__, context.getParams());

    auto results = context.getResults().initValues(std::min<unsigned>(context.getParams().getCount(),
                                                                      snapshotDecisionIds.size()));
    unsigned index = 0;
    while (!snapshotDecisionIds.empty()) {
        const auto& decision = snapshotDecisionIds.front()(vdb.db());
        auto decisionBuilder = results[index++].getValue();
        decisionBuilder.getId().setOperationId(decision.decisionId.instance.value);
        decisionBuilder.setCounted(firstDecisionWasCounted());
        snapshotDecisionIds.pop_front();
    }

    return kj::READY_NOW;
}

bool SnapshotDecisionGenerator::firstDecisionWasCounted() {
    // Decisions in the snapshot are stored in groups by voter, ordered from oldest to newest, and all of them are
    // valid, so if the second decision in the snapshot is from the same voter as the first, then the first was
    // superceded by the second and thus was not counted. If they have different voters, the first decision is the last
    // one by its voter, and so it was counted.
    if (snapshotDecisionIds.size() < 2)
        return true;
    const auto& d1 = snapshotDecisionIds[0](vdb.db());
    const auto& d2 = snapshotDecisionIds[1](vdb.db());
    return d1.voter != d2.voter;
}

} // namespace swv
