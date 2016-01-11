#ifndef CONTESTCREATIONREQUEST_HPP
#define CONTESTCREATIONREQUEST_HPP

#include <QObject>

#include <contestcreation.capnp.h>

#include "vendor/QQmlEnumClassHelper.h"

namespace swv {

class ContestCreationRequest : public QObject
{
    Q_OBJECT
public:
    using PurchaseRequest = capnp::Request<ContestCreator::PurchaseContestParams,
                                           ContestCreator::PurchaseContestResults>;

    ContestCreationRequest(PurchaseRequest&& request);
    virtual ~ContestCreationRequest() noexcept {}

private:
    PurchaseRequest request;
};

// Create an enum class for the LineItems
#define E(x) uint16_t(::ContestCreator::LineItems::x)
QML_ENUM_CLASS(LineItems,
               contestTypeOneOfN = E(CONTEST_TYPE_ONE_OF_N),
               pluralityTally = E(PLURALITY_TALLY),
               contestant3 = E(CONTESTANT3),
               contestant4 = E(CONTESTANT4),
               contestant5 = E(CONTESTANT5),
               contestant6 = E(CONTESTANT6),
               contestant7Plus = E(CONTESTANT7_PLUS),
               infiniteDurationContest = E(INFINITE_DURATION_CONTEST)
        )
#undef E

// Create an enum class for the ContestLimits
#define E(x) uint16_t(::ContestCreator::ContestLimits::x)
QML_ENUM_CLASS(ContestLimits,
               nameLength = E(NAME_LENGTH),
               descriptionSoftLength = E(DESCRIPTION_SOFT_LENGTH),
               descriptionHardLength = E(DESCRIPTION_HARD_LENGTH),
               contestantCount = E(CONTESTANT_COUNT),
               contestantName = E(CONTESTANT_NAME_LENGTH),
               contestantDescriptionSoftLength = E(CONTESTANT_DESCRIPTION_SOFT_LENGTH),
               contestantDescriptionHardLength = E(CONTESTANT_DESCRIPTION_HARD_LENGTH),
               maxEndDate = E(MAX_END_DATE)
        )
#undef E

#endif // CONTESTCREATIONREQUEST_HPP

} // namespace swv

Q_DECLARE_METATYPE(swv::LineItems)
Q_DECLARE_METATYPE(swv::ContestLimits)
