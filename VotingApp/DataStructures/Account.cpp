#include "Account.hpp"

namespace swv {
namespace data {

Account::Account(QObject *parent)
    : QObject(parent),
      m_balances(new QQmlVariantListModel(this))
{}

} } // namespace swv::data
