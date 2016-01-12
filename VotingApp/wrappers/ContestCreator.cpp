#include "ContestCreator.hpp"
#include "PurchaseContestRequest.hpp"

#include <kj/debug.h>

namespace swv {

void ContestCreator::taskFailed(kj::Exception&& exception)
{
    KJ_LOG(ERROR, exception);
    emit error(QString::fromStdString(exception.getDescription()));
}

ContestCreator::ContestCreator(::ContestCreator::Client&& creator)
    : creator(kj::mv(creator)),
      tasks(*this)
{
    refreshPrices();
    refreshLimits();
}


void ContestCreator::refreshPrices()
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

void ContestCreator::refreshLimits()
{
    auto promise = creator.getContestLimitsRequest().send();
    tasks.add(promise.then([this](capnp::Response<::ContestCreator::GetContestLimitsResults> r)
    {
        m_contestLimits.clear();
        for (auto limitKeyValue : r.getLimits().getEntries())
            m_priceSchedule[QString::number(static_cast<uint16_t>(limitKeyValue.getKey().getLimit()))] =
                    QVariant::fromValue(limitKeyValue.getValue().getValue());
        emit contestLimitsChanged(m_contestLimits);
    }));
}

PurchaseContestRequest* ContestCreator::getPurchaseContestRequest()
{
    refreshPrices();
    refreshLimits();
    return new PurchaseContestRequest(creator.purchaseContestRequest(), tasks, this);
}

} // namespace swv
