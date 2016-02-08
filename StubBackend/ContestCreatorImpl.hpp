#ifndef CONTESTCREATORIMPL_H
#define CONTESTCREATORIMPL_H

#include <contestcreator.capnp.h>

class ContestCreatorImpl : public ContestCreator::Server
{
public:
    ContestCreatorImpl();

protected:
    // ContestCreator::Server interface
    ::kj::Promise<void> getPriceSchedule(GetPriceScheduleContext context);
    ::kj::Promise<void> getContestLimits(GetContestLimitsContext context);
    ::kj::Promise<void> purchaseContest(PurchaseContestContext context);
};

#endif // CONTESTCREATORIMPL_H
