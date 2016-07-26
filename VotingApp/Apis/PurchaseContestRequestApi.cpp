#include "PurchaseContestRequestApi.hpp"
#include "PurchaseApi.hpp"
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
#define TEXT_GETTER_SETTER(property, requestField) \
    QString PurchaseContestRequestApi::property() const { \
        return convertText(request.asReader().getRequest().getContestOptions().get ## requestField ()); \
    } \
    void PurchaseContestRequestApi::set ## requestField(QString property) { \
        _CHECK_NOT_SAME(property) \
        request.getRequest().getContestOptions().set ## requestField(convertText(property)); \
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
    type PurchaseContestRequestApi::property() const { \
        _SIMPLE_GETTER_IMPL(ContestOptions().get ## requestField) \
    } \
    void PurchaseContestRequestApi::set ## upperProperty(type property) { \
        _SIMPLE_SETTER_IMPL(property, getContestOptions().set ## requestField(property)) \
    }

// Macro to generate getter and setter for simple properties in the sponsorship options
#define SPONSORSHIP_SIMPLE_GETTER_SETTER(property, upperProperty, type, requestField) \
    type PurchaseContestRequestApi::property() { \
        if (!sponsorshipEnabled()) \
            return {}; \
        _SIMPLE_GETTER_IMPL(Sponsorship().getOptions().get ## requestField) \
    } \
    void PurchaseContestRequestApi::set ## upperProperty(type property) { \
        if (!sponsorshipEnabled()) \
            setSponsorshipEnabled(true); \
        _SIMPLE_SETTER_IMPL(property, getSponsorship().getOptions().set ## requestField(property)) \
    }

namespace swv {
PurchaseContestRequestApi::PurchaseContestRequestApi(PurchaseRequest&& request, kj::TaskSet& taskTracker,
                                                     PromiseConverter& converter, QObject* parent)
    : QObject(parent),
      tasks(tasks),
      converter(converter),
      request(kj::mv(request))
{}

ContestType::Type PurchaseContestRequestApi::contestType() const {
    return static_cast<ContestType::Type>(request.asReader().getRequest().getContestOptions().getType());
}
void PurchaseContestRequestApi::setContestType(ContestType::Type type) {
    if (type == contestType())
        return;
    request.getRequest().getContestOptions().setType(static_cast<::Contest::Type>(type));
    emit contestTypeChanged(type);
}

TallyAlgorithm::Type PurchaseContestRequestApi::tallyAlgorithm() const {
    return static_cast<TallyAlgorithm::Type>(request.asReader().getRequest().getContestOptions().getTallyAlgorithm());
}
void PurchaseContestRequestApi::setTallyAlgorithm(TallyAlgorithm::Type algorithm) {
    if (algorithm == tallyAlgorithm())
        return;
    request.getRequest().getContestOptions().setTallyAlgorithm(static_cast<::Contest::TallyAlgorithm>(algorithm));
    emit tallyAlgorithmChanged(algorithm);
}

TEXT_GETTER_SETTER(name, Name)
TEXT_GETTER_SETTER(description, Description)
SIMPLE_GETTER_SETTER(weightCoin, WeightCoin, quint64, Coin)
SIMPLE_GETTER_SETTER(expiration, Expiration, qint64, EndTime)

bool PurchaseContestRequestApi::sponsorshipEnabled() const {
    return request.asReader().getRequest().getSponsorship().isOptions();
}

PurchaseApi* PurchaseContestRequestApi::submit() {
    updateContestants();
    KJ_LOG(DBG, "Submitting purchase request", request, request.getRequest().getContestOptions().getName());

    auto promise = request.send();
    auto purchase = promise.getPurchaseApi();

    return new PurchaseApi(kj::mv(purchase), converter, this);
}

void PurchaseContestRequestApi::setSponsorshipEnabled(bool enabled) {
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

void PurchaseContestRequestApi::updateContestants()
{
    auto target = request.getRequest().getContestOptions().initContestants().initEntries(m_contestants.count());
    for (uint i = 0; i < target.size(); ++i) {
        auto contestant = m_contestants.get(i).value<QObject*>();
        target[i].setKey(convertText(contestant->property("name").toString()));
        target[i].setValue(convertText(contestant->property("description").toString()));
    }
}

} // namespace swv
