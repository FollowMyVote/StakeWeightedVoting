#ifndef ACCOUNT_HPP
#define ACCOUNT_HPP

#include "vendor/QQmlVarPropertyHelpers.h"
#include "vendor/QQmlVariantListModel.h"

#include <QObject>

namespace swv { namespace data {

/*!
 * \qmltype AccountBalance
 * \instantiates swv::data::AccountBalance
 *
 * The AccountBalance type represents an account's total balance in a particular coin
 */
struct AccountBalance {
    Q_PROPERTY(quint64 coinId MEMBER coinId)
    Q_PROPERTY(qint64 amount MEMBER amount)
    quint64 coinId;
    qint64 amount;
    Q_GADGET
};

/*!
 * \qmltype Account
 * \instantiates swv::data::Account
 *
 * The Account type provides information about a specific account on the blockchain
 */
class Account : public QObject
{
    Q_OBJECT
    /*!
     * \qmlproperty Account::name
     * The name of the account
     */
    QML_READONLY_VAR_PROPERTY(QString, name)
    /*!
     * \qmlproperty Account::balances
     * The total balances controlled by the account. There is one entry per coin the account owns, and the amount on
     * each balance is the sum of all tokens the account holds in that coin. Each element is a AccountBalance object.
     *
     * \sa AccountBalance
     */
    QML_CONSTANT_VAR_PROPERTY(QQmlVariantListModel*, balances)
public:
    explicit Account(QObject *parent = 0);
};

} } // namespace swv::data

Q_DECLARE_METATYPE(swv::data::AccountBalance)

#endif // ACCOUNT_HPP
