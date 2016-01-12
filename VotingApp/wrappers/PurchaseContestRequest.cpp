#include "PurchaseContestRequest.hpp"

namespace swv {
PurchaseContestRequest::PurchaseContestRequest(PurchaseRequest&& request, kj::TaskSet& taskTracker, QObject* parent)
    : QObject(parent),
      tasks(taskTracker),
      request(kj::mv(request))
{
}
} // namespace swv
