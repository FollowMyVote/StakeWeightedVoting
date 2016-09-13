#ifndef DECISIONRECORD_HPP
#define DECISIONRECORD_HPP

#include <decision.capnp.h>

#include <QQmlVarPropertyHelpers.h>

#include <QObject>
#include <QVariantMap>

namespace swv {
namespace data {

/**
 * @brief The DecisionRecord class provides a QML-friendly representation of a DecisionRecord capnp structure
 *
 * This class copies the data from the DecisionRecord and makes it available to QML for reading. It never stores a
 * DecisionRecord reader or builder.
 */
class DecisionRecord : public QObject {
    Q_OBJECT
    QML_READONLY_VAR_PROPERTY(QString, id)
    QML_READONLY_VAR_PROPERTY(QString, voter)
    QML_READONLY_VAR_PROPERTY(qint64, weight)
    QML_READONLY_VAR_PROPERTY(QString, contestId)
    QML_READONLY_VAR_PROPERTY(QVariantMap, opinions)
    QML_READONLY_VAR_PROPERTY(QVariantList, writeIns)

public:
    explicit DecisionRecord(::DecisionRecord::Reader r = {}, QObject *parent = 0);
    virtual ~DecisionRecord() noexcept {}

    void updateFields(::DecisionRecord::Reader r);
};

} } // namespace swv::data
#endif // DECISIONRECORD_HPP
