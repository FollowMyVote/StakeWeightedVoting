/*
 * Copyright 2015 Follow My Vote, Inc.
 * This file is part of The Follow My Vote Stake-Weighted Voting Application ("SWV").
 *
 * SWV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SWV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SWV.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DECISION_HPP
#define DECISION_HPP

#include "vendor/QQmlVarPropertyHelpers.h"

#include <decision.capnp.h>

#include <capnp/message.h>

#include <QObject>
#include <QVariantMap>

namespace swv { namespace data {

/**
 * @brief The DecisionWrapper class is a read-write wrapper for the Decision type.
 */
class Decision : public QObject
{
    Q_OBJECT
    QML_READONLY_VAR_PROPERTY(QString, id)
    QML_READONLY_VAR_PROPERTY(QString, contestId)
    QML_WRITABLE_VAR_PROPERTY(QVariantMap, opinions)
    QML_WRITABLE_VAR_PROPERTY(QVariantList, writeIns)

public:
    Decision(::Decision::Reader r = {}, QObject* parent = nullptr);
    ~Decision() noexcept;

    void updateFields(::Decision::Reader r);
    void serialize(::Decision::Builder b);

    /// @brief Compare two decisions. Decisions are equal if they apply to the same contest and have the same opinions
    /// and write-ins. The IDs are not relevant to equality.
    bool operator== (const Decision& other) {
        return get_contestId() == other.get_contestId() &&
                get_opinions() == other.get_opinions() &&
                get_writeIns() == other.get_writeIns();
    }
    bool operator!= (const Decision& other) {
        return !(*this == other);
    }

private:
    /// Remove all opinions of zero
    QVariantMap canonicalizeOpinions(QVariantMap opinions);
    /// Update opinions from reader
    void updateOpinions(capnp::List<::Decision::Opinion>::Reader opinions);
    /// Update write-ins from reader
    void updateWriteIns(::Map<capnp::Text, capnp::Text>::Reader writeIns);
    /// Serialize opinions to builder
    void serializeOpinions(::Decision::Builder b);
    /// Serialize write-ins to builder
    void serializeWriteIns(::Decision::Builder b);
};

} } // namespace swv::data

#endif // DECISION_HPP
