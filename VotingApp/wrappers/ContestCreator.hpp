#ifndef CONTESTCREATOR_HPP
#define CONTESTCREATOR_HPP

#include <QObject>
#include <QVariantMap>

#include <contestcreator.capnp.h>

#include "vendor/QQmlEnumClassHelper.h"

namespace swv {
class PurchaseContestRequestWrapper;

class ContestCreatorWrapper : public QObject, private kj::TaskSet::ErrorHandler
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap priceSchedule READ priceSchedule NOTIFY priceScheduleChanged)
    Q_PROPERTY(QVariantMap contestLimits READ contestLimits NOTIFY contestLimitsChanged)

    QVariantMap m_priceSchedule;
    QVariantMap m_contestLimits;

    ::ContestCreator::Client creator;

    kj::TaskSet tasks;
    // ErrorHandler interface
    void taskFailed(kj::Exception&& exception);

public:
    explicit ContestCreatorWrapper(::ContestCreator::Client&& creator);
    virtual ~ContestCreatorWrapper() noexcept {}

    /// @brief Get the cached price schedule
    /// @note The result will be empty until the schedule is refreshed via @ref refreshPrices. Generally this will
    /// happen automatically, but make sure to connect to the changed signal
    ///
    /// The result is a sparse array where the indices correspond to LineItems enum values
    QVariantMap priceSchedule() {
        return m_priceSchedule;
    }
    /// @brief Get the cached contest limits
    /// @note The result will be empty until the limits are refreshed via @ref refreshLimits. Generally this will
    /// happen automatically, but make sure to connect to the changed signal
    ///
    /// The result is a sparse array where the indices correspond to ContestLimits enum values
    QVariantMap contestLimits() {
        return m_contestLimits;
    }

    /// @brief Get an empty contest purchase request
    Q_INVOKABLE swv::PurchaseContestRequestWrapper* getPurchaseContestRequest();

public slots:
    void refreshPrices();
    void refreshLimits();

signals:
    void priceScheduleChanged(QVariantMap priceSchedule);
    void contestLimitsChanged(QVariantMap contestLimits);

    void error(QString errorString);
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

} // namespace swv
#endif // CONTESTCREATOR_HPP
