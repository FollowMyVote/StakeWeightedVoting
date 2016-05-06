#include "PurchaseContestRequest.hpp"
#include "PurchaseWrapper.hpp"
#include "Converters.hpp"

#include <PromiseConverter.hpp>

/*
 * If it looks scary in here, that's because it is. Getting and setting from the capnp structs involves a lot of
 * boilerplate, which I've mostly tried to distil out into what turns out to be a fairly convoluted system of macros.
 * In hindsight, it may not have been any simpler to use the macros, but it works now, so I'm going with it. If the
 * requirements evolve and fixing the macros is too much of a pain, it may be simpler to just scrap them and have a ton
 * of redundant code.
 */

#define _CHECK_NOT_SAME(property) \
    if (property == this->property()) \
        return;

// Macro to generate getter and setter for text properties in the contestOptions
#define TEXT_GETTER_SETTER(property, upperProperty, requestField) \
    QString PurchaseContestRequestWrapper::property() const { \
        return convertText(request.asReader().getRequest().getContestOptions().get ## requestField ()); \
    } \
    void PurchaseContestRequestWrapper::set ## upperProperty(QString property) { \
        _CHECK_NOT_SAME(property) \
        request.getRequest().getContestOptions().set ## requestField(convertText(property)); \
        emit property ## Changed(property); \
    }

// Macro to generate getter and setter for enum properties in the contestOptions
#define ENUM_GETTER_SETTER(property, upperProperty) \
    upperProperty::Type PurchaseContestRequestWrapper::property() const { \
        return static_cast<upperProperty::Type>(request.asReader().getRequest().getContestOptions().get ## upperProperty ()); \
    } \
    void PurchaseContestRequestWrapper::set ## upperProperty(upperProperty::Type property) { \
        _CHECK_NOT_SAME(property) \
        request.getRequest().getContestOptions().set ## upperProperty (static_cast<::ContestCreator::upperProperty ## s>(property)); \
        emit property ## Changed(property); \
    }

#define _SIMPLE_GETTER_IMPL(requestField) \
    return request.asReader().getRequest().get ## requestField();
#define _SIMPLE_SETTER_IMPL(property, requestMethod) \
    _CHECK_NOT_SAME(property) \
    request.getRequest().requestMethod; \
    emit property ## Changed(property);

// Macro to generate getter and setter for properties in the contestOptions which require no special conversion steps
#define SIMPLE_GETTER_SETTER(property, upperProperty, type, requestField) \
    type PurchaseContestRequestWrapper::property() const { \
        _SIMPLE_GETTER_IMPL(ContestOptions().get ## requestField) \
    } \
    void PurchaseContestRequestWrapper::set ## upperProperty(type property) { \
        _SIMPLE_SETTER_IMPL(property, getContestOptions().set ## requestField(property)) \
    }

// Macro to generate getter and setter for simple properties in the sponsorship options
#define SPONSORSHIP_SIMPLE_GETTER_SETTER(property, upperProperty, type, requestField) \
    type PurchaseContestRequestWrapper::property() { \
        if (!sponsorshipEnabled()) \
            return {}; \
        _SIMPLE_GETTER_IMPL(Sponsorship().getOptions().get ## requestField) \
    } \
    void PurchaseContestRequestWrapper::set ## upperProperty(type property) { \
        if (!sponsorshipEnabled()) \
            setSponsorshipEnabled(true); \
        _SIMPLE_SETTER_IMPL(property, getSponsorship().getOptions().set ## requestField(property)) \
    }

namespace swv {
PurchaseContestRequestWrapper::PurchaseContestRequestWrapper(PurchaseRequest&& request,
                                                             kj::TaskSet& taskTracker,
                                                             QObject* parent)
    : QObject(parent),
      tasks(taskTracker),
      converter(tasks),
      request(kj::mv(request))
{}

ENUM_GETTER_SETTER(contestType, ContestType)
ENUM_GETTER_SETTER(tallyAlgorithm, TallyAlgorithm)
TEXT_GETTER_SETTER(name, Name, ContestName)
TEXT_GETTER_SETTER(description, Description, ContestDescription)
SIMPLE_GETTER_SETTER(weightCoin, WeightCoin, quint64, WeightCoin)
SIMPLE_GETTER_SETTER(expiration, Expiration, qint64, ContestExpiration)

bool PurchaseContestRequestWrapper::sponsorshipEnabled() const {
    return request.asReader().getRequest().getSponsorship().isOptions();
}

PurchaseWrapper* PurchaseContestRequestWrapper::submit() {
    updateContestants();
    KJ_LOG(DBG, "Submitting purchase request", request, request.getRequest().getContestOptions().getContestName());

    auto promise = request.send();
    auto purchase = promise.getPurchaseApi();

    return new PurchaseWrapper(kj::mv(purchase), tasks, this);
}

void PurchaseContestRequestWrapper::setSponsorshipEnabled(bool enabled) {
    if (enabled == sponsorshipEnabled())
        return;
    if (enabled)
        request.getRequest().initSponsorship().initOptions();
    else
        request.getRequest().initSponsorship().setNoSponsorship();
    emit sponsorshipEnabledChanged(enabled);
}

SPONSORSHIP_SIMPLE_GETTER_SETTER(sponsorMaxVotes, SponsorMaxVotes, qint64, MaxVotes)
SPONSORSHIP_SIMPLE_GETTER_SETTER(sponsorMaxRevotes, SponsorMaxRevotes, qint32, MaxRevotes)
SPONSORSHIP_SIMPLE_GETTER_SETTER(sponsorEndDate, SponsorEndDate, qint64, EndDate)
SPONSORSHIP_SIMPLE_GETTER_SETTER(sponsorIncentive, SponsorIncentive, qint64, Incentive)

void PurchaseContestRequestWrapper::updateContestants()
{
    auto target = request.getRequest().getContestOptions().initContestants().initEntries(m_contestants.count());
    for (uint i = 0; i < target.size(); ++i) {
        auto contestant = m_contestants.get(i).value<QObject*>();
        target[i].setKey(convertText(contestant->property("name").toString()));
        target[i].setValue(convertText(contestant->property("description").toString()));
    }
}

} // namespace swv
