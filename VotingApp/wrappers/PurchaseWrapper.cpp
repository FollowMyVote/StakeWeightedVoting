#include "PurchaseWrapper.hpp"
#include "Converters.hpp"

namespace swv {

void PurchaseWrapper::setComplete(bool complete)
{
    if (complete == m_complete) return;
    m_complete = complete;
    emit completeChanged(complete);
}

PurchaseWrapper::PurchaseWrapper(Purchase::Client&& api, kj::TaskSet& tasks, QObject *parent)
    : QObject(parent),
      api(kj::mv(api)),
      tasks(tasks),
      converter(tasks)
{
    class CompleteNotifier : public Notifier<capnp::Text>::Server {
        PurchaseWrapper& wrapper;
        virtual ::kj::Promise<void> notify(NotifyContext context) {
            wrapper.setComplete(context.getParams().getMessage() == "true");
            return kj::READY_NOW;
        }

    public:
        CompleteNotifier(PurchaseWrapper& wrapper) : wrapper(wrapper) {}
    };

    auto request = api.subscribeRequest();
    request.setNotifier(kj::heap<CompleteNotifier>(*this));
    tasks.add(request.send().then([](capnp::Response<Purchase::SubscribeResults>){}));
    tasks.add(api.completeRequest().send().then([this] (capnp::Response<Purchase::CompleteResults> r) {
                  setComplete(r.getResult());
              }));
}

PurchaseWrapper::~PurchaseWrapper() noexcept
{}

Promise* PurchaseWrapper::prices()
{
    return converter.convert(api.pricesRequest().send(), [](capnp::Response<Purchase::PricesResults> r) {
        QVariantList results;
        for (auto price : r.getPrices()) {
            results.append(QVariantMap{{"coinId", QVariant::fromValue(price.getCoinId())},
                                       {"amount", QVariant::fromValue(price.getAmount())},
                                       {"payAddress", QVariant::fromValue(convertText(price.getPayAddress()))}});
        }
        return QVariantList() << results;
    });
}

void PurchaseWrapper::paymentSent(qint16 selectedPrice)
{
    auto request = api.paymentSentRequest();
    request.setSelectedPrice(selectedPrice);
    tasks.add(request.send().then([](capnp::Response<Purchase::PaymentSentResults>){}));
}

} // namespace swv
