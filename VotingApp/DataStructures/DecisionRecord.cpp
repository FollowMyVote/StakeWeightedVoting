#include "DecisionRecord.hpp"
#include "Decision.hpp"
#include "Converters.hpp"

namespace swv {
namespace data {

DecisionRecord::DecisionRecord(::DecisionRecord::Reader r, QObject *parent)
    : QObject(parent) {
    updateFields(r);
}

void DecisionRecord::updateFields(::DecisionRecord::Reader r) {
    update_id(convertBlob(ReaderPacker(r.getId()).array()).toHex());
    update_voter(QString::fromStdString(r.getVoter()));
    update_weight(r.getWeight());

    // Decision has a fair bit of code to parse the opinions and writeins. Just use that rather than rewrite it here.
    swv::data::Decision d(r.getDecision());
    update_contestId(d.get_contestId());
    update_opinions(d.get_opinions());
    update_writeIns(d.get_writeIns());
}

} } // namespace swv::data
