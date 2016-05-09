#ifndef CONTESTCREATIONREQUEST_HPP
#define CONTESTCREATIONREQUEST_HPP

#include <QObject>

#include <PromiseConverter.hpp>
#include <contestcreator.capnp.h>

#include "vendor/QQmlEnumClassHelper.h"
#include "vendor/QQmlObjectListModel.h"
#include "vendor/QQmlVariantListModel.h"

namespace swv {
class PurchaseWrapper;

QML_ENUM_CLASS(ContestType,
               OneOfN = static_cast<uint16_t>(::Contest::Type::ONE_OF_N)
        )
QML_ENUM_CLASS(TallyAlgorithm,
               Plurality = static_cast<uint16_t>(::Contest::TallyAlgorithm::PLURALITY)
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
    Q_PROPERTY(swv::ContestType::Type contestType READ contestType WRITE setContestType NOTIFY contestTypeChanged)
    Q_PROPERTY(swv::TallyAlgorithm::Type tallyAlgorithm READ tallyAlgorithm WRITE setTallyAlgorithm
               NOTIFY tallyAlgorithmChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QQmlVariantListModel* contestants READ contestants CONSTANT)
    Q_PROPERTY(quint64 weightCoin READ weightCoin WRITE setWeightCoin NOTIFY weightCoinChanged)
    Q_PROPERTY(qint64 expiration READ expiration WRITE setExpiration NOTIFY expirationChanged)
    Q_PROPERTY(bool sponsorshipEnabled READ sponsorshipEnabled WRITE setSponsorshipEnabled
               NOTIFY sponsorshipEnabledChanged)
    Q_PROPERTY(qint64 sponsorMaxVotes READ sponsorMaxVotes WRITE setSponsorMaxVotes NOTIFY sponsorMaxVotesChanged)
    Q_PROPERTY(qint32 sponsorMaxRevotes READ sponsorMaxRevotes WRITE setSponsorMaxRevotes
               NOTIFY sponsorMaxRevotesChanged)
    Q_PROPERTY(qint64 sponsorEndDate READ sponsorEndDate WRITE setSponsorEndDate NOTIFY sponsorEndDateChanged)
    Q_PROPERTY(qint64 sponsorIncentive READ sponsorIncentive WRITE setSponsorIncentive NOTIFY sponsorIncentiveChanged)

    kj::TaskSet& tasks;
    QQmlVariantListModel m_contestants;
    PromiseConverter converter;

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
    QQmlVariantListModel* contestants() {
        return &m_contestants;
    }
    quint64 weightCoin() const;
    qint64 expiration() const;
    ContestType::Type contestType() const;
    TallyAlgorithm::Type tallyAlgorithm() const;
    bool sponsorshipEnabled() const;
    qint64 sponsorMaxVotes();
    qint32 sponsorMaxRevotes();
    qint64 sponsorEndDate();
    qint64 sponsorIncentive();

public slots:
    /// @brief Submit the request to the server. This consumes the request.
    /// @return A map with two entries: "purchaseApi" (a purchase API wrapper) and "surchargePromise" (a @ref Promise
    /// for the surcharge list, which is itself a QVariantList of objects with "description" and "charge" fields)
    swv::PurchaseWrapper* submit();

    void setName(QString name);
    void setDescription(QString description);
    void setWeightCoin(quint64 weightCoin);
    void setExpiration(qint64 expiration);
    void setContestType(ContestType::Type type);
    void setTallyAlgorithm(TallyAlgorithm::Type algorithm);
    void setSponsorshipEnabled(bool enabled);
    void setSponsorMaxVotes(qint64 sponsorMaxVotes);
    void setSponsorMaxRevotes(qint32 sponsorMaxRevotes);
    void setSponsorEndDate(qint64 sponsorEndDate);
    void setSponsorIncentive(qint64 sponsorIncentive);

    signals:
    void nameChanged(QString name);
    void descriptionChanged(QString description);
    void weightCoinChanged(quint64 weightCoin);
    void expirationChanged(qint64 expiration);
    void contestTypeChanged(ContestType::Type contestType);
    void tallyAlgorithmChanged(TallyAlgorithm::Type tallyAlgorithm);
    void sponsorshipEnabledChanged(bool sponsorshipEnabled);
    void sponsorMaxVotesChanged(qint64 sponsorMaxVotes);
    void sponsorMaxRevotesChanged(qint32 sponsorMaxRevotes);
    void sponsorEndDateChanged(qint64 sponsorEndDate);
    void sponsorIncentiveChanged(qint64 sponsorIncentive);

protected slots:
    void updateContestants();

private:
    PurchaseRequest request;
};

} // namespace swv
#endif // CONTESTCREATIONREQUEST_HPP
