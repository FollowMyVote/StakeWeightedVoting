#include "ContestCreatorApi.hpp"
#include "PurchaseContestRequestApi.hpp"

#include <kj/debug.h>

#include <QDebug>

namespace swv {

void ContestCreatorApi::taskFailed(kj::Exception&& exception)
{
    KJ_LOG(ERROR, exception);
    emit error(QString::fromStdString(exception.getDescription()));
}

ContestCreatorApi::ContestCreatorApi(::ContestCreator::Client&& creator)
    : creator(kj::mv(creator)),
      tasks(*this)
{
    refreshPrices();
    refreshLimits();
}


void ContestCreatorApi::refreshPrices()
{
    auto promise = creator.getPriceScheduleRequest().send();
    tasks.add(promise.then([this](capnp::Response<::ContestCreator::GetPriceScheduleResults> r)
    {
        m_priceSchedule.clear();
        for (auto lineItemPrice : r.getSchedule().getEntries())
            m_priceSchedule[QString::number(static_cast<uint16_t>(lineItemPrice.getKey().getItem()))] =
                    QVariant::fromValue(lineItemPrice.getValue().getPrice());
        emit priceScheduleChanged(m_priceSchedule);
    }));
}

void ContestCreatorApi::refreshLimits()
{
    auto promise = creator.getContestLimitsRequest().send();
    tasks.add(promise.then([this](capnp::Response<::ContestCreator::GetContestLimitsResults> r)
    {
        m_contestLimits.clear();
        for (auto limitKeyValue : r.getLimits().getEntries())
            m_contestLimits[QString::number(static_cast<uint16_t>(limitKeyValue.getKey().getLimit()))] =
                    QVariant::fromValue(limitKeyValue.getValue().getValue());
        emit contestLimitsChanged(m_contestLimits);
    }));
}

PurchaseContestRequestApi* ContestCreatorApi::getPurchaseContestRequest()
{
    refreshPrices();
    refreshLimits();
    return new PurchaseContestRequestApi(creator.purchaseContestRequest(), tasks, this);
}

} // namespace swv
