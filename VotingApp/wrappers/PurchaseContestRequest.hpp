#ifndef CONTESTCREATIONREQUEST_HPP
#define CONTESTCREATIONREQUEST_HPP

#include <QObject>

#include <contestcreator.capnp.h>

namespace swv {

/**
 * @brief The PurchaseContestRequest class provies a QML interface to a PurchaseRequest for a contest
 *
 * The request is initially empty, and must be configured by setting its properties. When the request is configured, it
 * can be submitted by calling @ref submit.
 */
class PurchaseContestRequest : public QObject
{
    Q_OBJECT
public:
    using PurchaseRequest = capnp::Request<::ContestCreator::PurchaseContestParams,
                                           ::ContestCreator::PurchaseContestResults>;

    PurchaseContestRequest(PurchaseRequest&& request);
    virtual ~PurchaseContestRequest() noexcept {}

public slots:
    /// @brief Submit the request to the server. This consumes the request.
    void submit(){}

private:
    PurchaseRequest request;
};

} // namespace swv
#endif // CONTESTCREATIONREQUEST_HPP
