#include "PurchaseContestRequest.hpp"

namespace swv {
PurchaseContestRequest::PurchaseContestRequest(PurchaseRequest&& request)
    : request(kj::mv(request))
{
}
} // namespace swv
