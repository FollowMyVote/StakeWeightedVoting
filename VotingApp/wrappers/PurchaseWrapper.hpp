#ifndef PURCHASEWRAPPER_HPP
#define PURCHASEWRAPPER_HPP

#include <purchase.capnp.h>
#include <PromiseConverter.hpp>

#include <QObject>

namespace swv {

/**
 * @brief The PurchaseWrapper class provides a QML-friendly interface to the CapnP Purchase type
 *
 * PurchaseWrapper subscribes to a push notification from the server when the purchase is complete, so the complete
 * property will be updated automatically as soon as the purchase completes.
 */
class PurchaseWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool complete READ complete NOTIFY completeChanged)

    Purchase::Client api;
    kj::TaskSet& tasks;
    PromiseConverter converter;
    bool m_complete = false;

    void setComplete(bool complete);

public:
    explicit PurchaseWrapper(Purchase::Client&& api, kj::TaskSet& tasks, QObject *parent = 0);
    virtual ~PurchaseWrapper() noexcept;

    bool complete() const {
        return m_complete;
    }
    Q_INVOKABLE Promise* prices();

public slots:
    /**
     * @brief Notify the server that payment has been sent
     * @param selectedPrice Index in array returned from @ref prices of the price which was paid
     */
    void paymentSent(qint16 selectedPrice);

signals:
    void completeChanged(bool complete);
};

} // namespace swv
#endif // PURCHASEWRAPPER_HPP
