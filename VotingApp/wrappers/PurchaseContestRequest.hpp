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

    kj::TaskSet& tasks;
public:
    using PurchaseRequest = capnp::Request<::ContestCreator::PurchaseContestParams,
                                           ::ContestCreator::PurchaseContestResults>;

    /**
     * @param request The request to complete/submit
     * @param taskTracker The task set to add the submission promise to
     * @param parent The QObject parent must be set to an object which will not outlive taskTracker
     */
    PurchaseContestRequest(PurchaseRequest&& request, kj::TaskSet& taskTracker, QObject* parent);
    virtual ~PurchaseContestRequest() noexcept {}

public slots:
    /// @brief Submit the request to the server. This consumes the request.
    void submit(){}

private:
    PurchaseRequest request;
};

} // namespace swv
#endif // CONTESTCREATIONREQUEST_HPP
