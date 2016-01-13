#include "PurchaseContestRequest.hpp"
#include "Converters.hpp"

#define TEXT_GETTER(requestField) \
    return convertText(request.asReader().getRequest().get ## requestField ())

#define TEXT_SETTER(property, requestField) \
    if (property == this->property()) \
    return; \
    convertText(request.getRequest().get ## requestField (), property); \
    emit property ## Changed(property)

#define TEXT_GETTER_SETTER(property, upperProperty, requestField) \
    QString PurchaseContestRequestWrapper::property() const { \
    TEXT_GETTER(requestField); \
    } \
    void PurchaseContestRequestWrapper::set ## upperProperty(QString property) { \
    TEXT_SETTER(property, requestField); \
    }

namespace swv {
PurchaseContestRequestWrapper::PurchaseContestRequestWrapper(PurchaseRequest&& request, kj::TaskSet& taskTracker, QObject* parent)
    : QObject(parent),
      tasks(taskTracker),
      request(kj::mv(request))
{
}

TEXT_GETTER_SETTER(name, Name, ContestName)
TEXT_GETTER_SETTER(description, Description, ContestDescription)
} // namespace swv
