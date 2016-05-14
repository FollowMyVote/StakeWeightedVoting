#include "PurchaseWrapper.hpp"
#include "Converters.hpp"

#include <QDebug>

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
            wrapper.setComplete(context.getParams().getNotification() == "true");
            return kj::READY_NOW;
        }

    public:
        CompleteNotifier(PurchaseWrapper& wrapper) : wrapper(wrapper) {}
    };

    auto request = this->api.subscribeRequest();
    request.setNotifier(kj::heap<CompleteNotifier>(*this));
    tasks.add(request.send().then([](capnp::Response<Purchase::SubscribeResults>){}));
    tasks.add(this->api.completeRequest().send().then([this] (capnp::Response<Purchase::CompleteResults> r) {
                  setComplete(r.getResult());
              }));
}

PurchaseWrapper::~PurchaseWrapper() noexcept
{}

Promise* PurchaseWrapper::prices(QStringList promoCodes)
{
    auto request = api.pricesRequest();
    auto codes = request.initPromoCodes(promoCodes.size());

    for (int i = 0; i < codes.size(); ++i)
        codes.set(i, convertText(promoCodes[i]));

    return converter.convert(request.send(), [](capnp::Response<Purchase::PricesResults> r) {
        QVariantList totals;
        for (auto price : r.getPrices())
            totals.append(QVariantMap{{"coinId", QVariant::fromValue(qreal(price.getCoinId()))},
                                       {"amount", QVariant::fromValue(qreal(price.getAmount()))},
                                       {"payAddress", QVariant::fromValue(convertText(price.getPayAddress()))}});
        QVariantList adjustments;
        for (auto adjustment : r.getAdjustments().getEntries())
            adjustments.append(QVariantMap{{"reason", QVariant::fromValue(convertText(adjustment.getKey()))},
                                           {"amount", QVariant::fromValue(qreal(adjustment.getValue().getPrice()))}});
        return QVariantList() << QVariant::fromValue(totals) << QVariant::fromValue(adjustments);
    });
}

void PurchaseWrapper::paymentSent(qint16 selectedPrice)
{
    auto request = api.paymentSentRequest();
    request.setSelectedPrice(selectedPrice);
    tasks.add(request.send().then([](capnp::Response<Purchase::PaymentSentResults>){}));
}

} // namespace swv
