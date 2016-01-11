#include "ContestCreationRequest.hpp"

namespace swv {
ContestCreationRequest::ContestCreationRequest(PurchaseRequest&& request)
    : request(kj::mv(request))
{
}
} // namespace swv
