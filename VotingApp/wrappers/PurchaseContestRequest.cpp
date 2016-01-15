#include "PurchaseContestRequest.hpp"
#include "Converters.hpp"

#define _CHECK_NOT_SAME(property) \
    if (property == this->property()) \
        return;

// Macro to generate getter and setter for text properties
#define TEXT_GETTER_SETTER(property, upperProperty, requestField) \
    QString PurchaseContestRequestWrapper::property() const { \
        return convertText(request.asReader().getRequest().get ## requestField ()); \
    } \
    void PurchaseContestRequestWrapper::set ## upperProperty(QString property) { \
        _CHECK_NOT_SAME(property) \
        convertText(request.getRequest().get ## requestField (), property); \
        emit property ## Changed(property); \
    }

// Macro to generate getter and setter for enum properties
#define ENUM_GETTER_SETTER(property, upperProperty) \
    upperProperty::Type PurchaseContestRequestWrapper::property() const { \
        return static_cast<upperProperty::Type>(request.asReader().getRequest().get ## upperProperty ()); \
    } \
    void PurchaseContestRequestWrapper::set ## upperProperty(upperProperty::Type property) { \
        _CHECK_NOT_SAME(property) \
        request.getRequest().set ## upperProperty (static_cast<::ContestCreator::upperProperty ## s>(property)); \
        emit property ## Changed(property); \
    }

#define _SIMPLE_GETTER_IMPL(requestField) \
    return request.asReader().getRequest().get ## requestField();
#define _SIMPLE_SETTER_IMPL(property, requestMethod) \
    _CHECK_NOT_SAME(property) \
    request.getRequest().requestMethod; \
    emit property ## Changed(property);

// Macro to generate getter and setter for properties which require no special conversion steps
#define SIMPLE_GETTER_SETTER(property, upperProperty, type, requestField) \
    type PurchaseContestRequestWrapper::property() const { \
        _SIMPLE_GETTER_IMPL(requestField) \
    } \
    void PurchaseContestRequestWrapper::set ## upperProperty(type property) { \
        _SIMPLE_SETTER_IMPL(property, set ## requestField(property)) \
    }

// Macro to generate getter and setter for simple properties in the sponsorship options
#define SPONSORSHIP_SIMPLE_GETTER_SETTER(property, upperProperty, type, requestField) \
    type PurchaseContestRequestWrapper::property() { \
        _SIMPLE_GETTER_IMPL(Sponsorship().getOptions().get ## requestField) \
    } \
    void PurchaseContestRequestWrapper::set ## upperProperty(type property) { \
        if (!sponsorshipEnabled()) \
            request.getRequest().initSponsorship().initOptions(); \
        _SIMPLE_SETTER_IMPL(property, getSponsorship().getOptions().set ## requestField(property)) \
    }

namespace swv {
PurchaseContestRequestWrapper::PurchaseContestRequestWrapper(PurchaseRequest&& request,
                                                             kj::TaskSet& taskTracker,
                                                             QObject* parent)
    : QObject(parent),
      tasks(taskTracker),
      request(kj::mv(request))
{
    // When the contestants change, automatically update the request
    connect(&m_contestants, &QQmlVariantListModel::dataChanged,
            this, &PurchaseContestRequestWrapper::updateContestants);
}

ENUM_GETTER_SETTER(contestType, ContestType)
ENUM_GETTER_SETTER(tallyAlgorithm, TallyAlgorithm)
TEXT_GETTER_SETTER(name, Name, ContestName)
TEXT_GETTER_SETTER(description, Description, ContestDescription)
SIMPLE_GETTER_SETTER(weightCoin, WeightCoin, quint64, WeightCoin)
SIMPLE_GETTER_SETTER(expiration, Expiration, qint64, ContestExpiration)

bool PurchaseContestRequestWrapper::sponsorshipEnabled() const {
    return request.asReader().getRequest().getSponsorship().isOptions();
}

void PurchaseContestRequestWrapper::submit() {
    //TODO: implement me
}

void PurchaseContestRequestWrapper::disableSponsorship() {
    request.getRequest().initSponsorship().setNoSponsorship();
}

SPONSORSHIP_SIMPLE_GETTER_SETTER(sponsorMaxVotes, SponsorMaxVotes, qint64, MaxVotes)
SPONSORSHIP_SIMPLE_GETTER_SETTER(sponsorMaxRevotes, SponsorMaxRevotes, qint32, MaxRevotes)
SPONSORSHIP_SIMPLE_GETTER_SETTER(sponsorEndDate, SponsorEndDate, qint64, EndDate)
SPONSORSHIP_SIMPLE_GETTER_SETTER(sponsorIncentive, SponsorIncentive, qint64, Incentive)

// Converts a QQmlVariantListModel to a capnp list. Func is a callable taking an element of List and a QVariant as
// arguments which copies the QVariant into the List element
template <typename List, typename Func>
void updateList(List target, const QQmlVariantListModel& source, Func copier) {
    for (uint i = 0; i < target.size(); ++i)
        copier(target[i], source.get(i).toMap());
}

void PurchaseContestRequestWrapper::updateContestants()
{
    updateList(request.getRequest().initContestants().initEntries(m_contestants.count()), m_contestants,
               [] (::Map<capnp::Text, capnp::Text>::Entry::Builder dest, const QVariant& src) {
        auto srcMap = src.toMap();
        convertText(dest.getKey(), srcMap["name"].toString());
        convertText(dest.getValue(), srcMap["description"].toString());
    });
}

void PurchaseContestRequestWrapper::updatePromoCodes()
{
    updateList(request.getRequest().initPromoCodes(m_promoCodes.count()), m_promoCodes,
               [] (capnp::Text::Builder dest, const QVariant& src) {
        convertText(dest, src.toString());
    });
}
} // namespace swv
