#include "PurchaseContestRequest.hpp"
#include "Converters.hpp"

// Macro to generate getter and setter for text properties
#define TEXT_GETTER_SETTER(property, upperProperty, requestField) \
    QString PurchaseContestRequestWrapper::property() const { \
        return convertText(request.asReader().getRequest().get ## requestField ()); \
    } \
    void PurchaseContestRequestWrapper::set ## upperProperty(QString property) { \
        if (property == this->property()) \
        return; \
        convertText(request.getRequest().get ## requestField (), property); \
        emit property ## Changed(property); \
    }

// Macro to generate getter and setter for enum properties
#define ENUM_GETTER_SETTER(property, upperProperty) \
    upperProperty::Type PurchaseContestRequestWrapper::property() const { \
        return static_cast<upperProperty::Type>(request.asReader().getRequest().get ## upperProperty ()); \
    } \
    void PurchaseContestRequestWrapper::set ## upperProperty(upperProperty::Type property) { \
        if (property == this->property()) \
            return; \
        request.getRequest().set ## upperProperty (static_cast<::ContestCreator::upperProperty ## s>(property)); \
        emit property ## Changed(property); \
    }

// Macro to generate getter and setter for properties which require no special conversion steps
#define SIMPLE_GETTER_SETTER(property, upperProperty, type, requestField) \
    type PurchaseContestRequestWrapper::property() const { \
        return request.asReader().getRequest().get ## requestField(); \
    } \
    void PurchaseContestRequestWrapper::set ## upperProperty(type property) { \
        if (property == this->property()) \
            return; \
        request.getRequest().set ## requestField(property); \
        emit property ## Changed(property); \
    }

namespace swv {
PurchaseContestRequestWrapper::PurchaseContestRequestWrapper(PurchaseRequest&& request, kj::TaskSet& taskTracker, QObject* parent)
    : QObject(parent),
      tasks(taskTracker),
      request(kj::mv(request))
{
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
void PurchaseContestRequestWrapper::disableSponsorship() {
    request.getRequest().initSponsorship().setNoSponsorship();
}
} // namespace swv
