#ifndef CONTESTCREATIONREQUEST_HPP
#define CONTESTCREATIONREQUEST_HPP

#include <QObject>

#include <contestcreator.capnp.h>

#include "vendor/QQmlEnumClassHelper.h"

namespace swv {

QML_ENUM_CLASS(ContestType,
               OneOfN = static_cast<uint16_t>(::ContestCreator::ContestTypes::ONE_OF_N)
        )
QML_ENUM_CLASS(TallyAlgorithm,
               Plurality = static_cast<uint16_t>(::ContestCreator::TallyAlgorithms::PLURALITY)
        )

/**
 * @brief The PurchaseContestRequest class provies a QML interface to manage purchasing a contest
 *
 * The PurchaseContestRequest properties map to the fields of the ContestCreationRequest capnp struct; however, this
 * class is really a wrapper for a request to call purchaseContest on the ContestCreator API. When the request is
 * completely configured, it can be submitted by calling @ref submit, which will return a promise for a
 * PurchaseContestResponse.
 */
class PurchaseContestRequestWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ContestType::Type contestType READ contestType WRITE setContestType NOTIFY contestTypeChanged)
    Q_PROPERTY(TallyAlgorithm::Type tallyAlgorithm READ tallyAlgorithm WRITE setTallyAlgorithm
               NOTIFY tallyAlgorithmChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(quint64 weightCoin READ weightCoin WRITE setWeightCoin NOTIFY weightCoinChanged)
    Q_PROPERTY(qint64 expiration READ expiration WRITE setExpiration NOTIFY expirationChanged)

    kj::TaskSet& tasks;
public:
    using PurchaseRequest = capnp::Request<::ContestCreator::PurchaseContestParams,
                                           ::ContestCreator::PurchaseContestResults>;

    /**
     * @param request The request to complete/submit
     * @param taskTracker The task set to add the submission promise to
     * @param parent The QObject parent must be set to an object which will not outlive taskTracker
     */
    PurchaseContestRequestWrapper(PurchaseRequest&& request, kj::TaskSet& taskTracker, QObject* parent);
    virtual ~PurchaseContestRequestWrapper() noexcept {}

    QString name() const;
    QString description() const;
    quint64 weightCoin() const
    {
        return request.asReader().getRequest().getWeightCoin();
    }
    qint64 expiration() const
    {
        return request.asReader().getRequest().getContestExpiration();
    }
    ContestType::Type contestType() const;
    TallyAlgorithm::Type tallyAlgorithm() const;

public slots:
    /// @brief Submit the request to the server. This consumes the request.
    void submit(){}

    void setName(QString name);
    void setDescription(QString description);
    void setWeightCoin(quint64 weightCoin)
    {
        if (weightCoin == this->weightCoin())
            return;

        request.getRequest().setWeightCoin(weightCoin);
        emit weightCoinChanged(weightCoin);
    }
    void setExpiration(qint64 expiration)
    {
        if (expiration == this->expiration())
            return;

        request.getRequest().setContestExpiration(expiration);
        emit expirationChanged(expiration);
    }
    void setContestType(ContestType::Type contestType);
    void setTallyAlgorithm(TallyAlgorithm::Type tallyAlgorithm);

signals:
    void nameChanged(QString name);
    void descriptionChanged(QString description);
    void weightCoinChanged(quint64 weightCoin);
    void expirationChanged(qint64 expiration);
    void contestTypeChanged(ContestType::Type contestType);
    void tallyAlgorithmChanged(TallyAlgorithm::Type tallyAlgorithm);

private:
    PurchaseRequest request;
};

} // namespace swv
#endif // CONTESTCREATIONREQUEST_HPP
