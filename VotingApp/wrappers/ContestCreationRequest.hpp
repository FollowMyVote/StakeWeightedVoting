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

// Create an enum class which mirrors LineItems in contestcreation.capnp
#define E(x) uint16_t(::ContestCreator::LineItems::x)
QML_ENUM_CLASS(LineItems,
               ContestTypeOneOfN = E(CONTEST_TYPE_ONE_OF_N),
               PluralityTally = E(PLURALITY_TALLY),
               Contestant3 = E(CONTESTANT3),
               Contestant4 = E(CONTESTANT4),
               Contestant5 = E(CONTESTANT5),
               Contestant6 = E(CONTESTANT6),
               Contestant7Plus = E(CONTESTANT7_PLUS),
               InfiniteDurationContest = E(INFINITE_DURATION_CONTEST)
        )
#undef E

// Create an enum class which mirrors ContestLimits in contestcreation.capnp
#define E(x) uint16_t(::ContestCreator::ContestLimits::x)
QML_ENUM_CLASS(ContestLimits,
               NameLength = E(NAME_LENGTH),
               DescriptionSoftLength = E(DESCRIPTION_SOFT_LENGTH),
               DescriptionHardLength = E(DESCRIPTION_HARD_LENGTH),
               ContestantCount = E(CONTESTANT_COUNT),
               ContestantNameLength = E(CONTESTANT_NAME_LENGTH),
               ContestantDescriptionSoftLength = E(CONTESTANT_DESCRIPTION_SOFT_LENGTH),
               ContestantDescriptionHardLength = E(CONTESTANT_DESCRIPTION_HARD_LENGTH),
               MaxEndDate = E(MAX_END_DATE)
        )
#undef E

#endif // CONTESTCREATIONREQUEST_HPP

} // namespace swv

Q_DECLARE_METATYPE(swv::LineItems)
Q_DECLARE_METATYPE(swv::ContestLimits)
