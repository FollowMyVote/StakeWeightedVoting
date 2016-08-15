#include "PurchaseApi.hpp"
#include "Converters.hpp"

#include <QDebug>

namespace swv {

void PurchaseApi::setComplete(bool success) {
    if (success == m_complete) return;
    KJ_LOG(DBG, "Purchase complete", success);
    m_complete = success;
    emit isCompleteChanged(success);
    emit purchaseConfirmed();
}

PurchaseApi::PurchaseApi(Purchase::Client&& api, PromiseConverter& converter, QObject *parent)
    : QObject(parent),
      api(kj::mv(api)),
      converter(converter) {
    class CompleteNotifier : public Notifier<capnp::Text>::Server {
        PurchaseApi& wrapper;
        virtual ::kj::Promise<void> notify(NotifyContext context) {
            if (context.getParams().getNotification() != "true") {
                KJ_LOG(ERROR, "Purchase failed...");
                emit wrapper.purchaseFailed();
            }
            wrapper.setComplete(true);
            return kj::READY_NOW;
        }

    public:
        CompleteNotifier(PurchaseApi& wrapper) : wrapper(wrapper) {}
    };

    auto request = this->api.subscribeRequest();
    request.setNotifier(kj::heap<CompleteNotifier>(*this));
    converter.adopt(request.send().then([](capnp::Response<Purchase::SubscribeResults>){}));
    converter.adopt(this->api.completeRequest().send().then([this] (capnp::Response<Purchase::CompleteResults> r) {
        setComplete(r.getResult());
    }));
}

PurchaseApi::~PurchaseApi() noexcept
{}

QJSValue PurchaseApi::prices(QStringList promoCodes) {
    auto request = api.pricesRequest();
    auto codes = request.initPromoCodes(promoCodes.size());

    for (auto i = 0u; i < codes.size(); ++i)
        codes.set(i, convertText(promoCodes[i]));

    return converter.convert(request.send(), [](capnp::Response<Purchase::PricesResults> r) {
        QVariantList totals;
        for (auto price : r.getPrices())
            totals.append(QVariantMap{{"coinId", QVariant::fromValue(qreal(price.getCoinId()))},
                                      {"amount", QVariant::fromValue(qreal(price.getAmount()))},
                                      {"payAddress", QVariant::fromValue(convertText(price.getPayAddress()))},
                                      {"memo", QVariant::fromValue(convertText(price.getPaymentMemo()))}});
        QVariantList adjustments;
        for (auto adjustment : r.getAdjustments().getEntries())
            adjustments.append(QVariantMap{{"reason", QVariant::fromValue(convertText(adjustment.getKey()))},
                                           {"amount", QVariant::fromValue(qreal(adjustment.getValue().getPrice()))}});
        return QVariantMap{{"totals", totals}, {"adjustments", adjustments}};
    });
}

void PurchaseApi::paymentSent(qint16 selectedPrice) {
    auto request = api.paymentSentRequest();
    request.setSelectedPrice(selectedPrice);
    converter.adopt(request.send().then([](capnp::Response<Purchase::PaymentSentResults>){}));
}

} // namespace swv
