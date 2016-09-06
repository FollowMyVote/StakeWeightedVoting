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

#ifndef CONTEST_HPP
#define CONTEST_HPP

#include "Decision.hpp"

#include <contest.capnp.h>

#include <QQmlVarPropertyHelpers.h>

#include <QObject>
#include <QDateTime>

namespace swv { namespace data {

/**
 * @brief The Contest class is a QML-friendly presentation of the data in a capnp UnsignedContest
 *
 * In addition to exposing the properties of ::UnsignedContest in a QML-accessible form, Contest implements the concept
 * of the Current Decision for the contest. The current decision is the @ref swv::data::DecisionWrapper which should be
 * displayed in the UI as the decision on the contest.
 */
class Contest : public QObject {
private:
    Q_OBJECT
    QML_READONLY_VAR_PROPERTY(QString, id)
    QML_READONLY_VAR_PROPERTY(QString, name)
    QML_READONLY_VAR_PROPERTY(QString, description)
    QML_READONLY_VAR_PROPERTY(QVariantMap, tags)
    QML_READONLY_VAR_PROPERTY(QVariantList, contestants)
    QML_READONLY_VAR_PROPERTY(quint64, coin)
    QML_READONLY_VAR_PROPERTY(QDateTime, startTime)
    Q_PROPERTY(swv::data::Decision* pendingDecision READ pendingDecision WRITE setPendingDecision NOTIFY pendingDecisionChanged)
    Q_PROPERTY(swv::data::Decision* officialDecision READ officialDecision WRITE setOfficialDecision NOTIFY officialDecisionChanged)

    Decision* m_pendingDecision = nullptr;
    Decision* m_officialDecision = nullptr;

public:
    Contest(QString id = "00", QObject* parent = nullptr);
    virtual ~Contest();

    void updateFields(::Contest::Reader r);
    void serialize(::Contest::Builder b);

    Decision* pendingDecision() const {
        return m_pendingDecision;
    }
    Decision* officialDecision() const {
        return m_officialDecision;
    }

public slots:
    /// @brief Set the pending decision. Destroys the old pending decision and takes ownership of the new one.
    void setPendingDecision(Decision* newDecision);
    /// @brief Set the official decision. Destroys the old official decision and takes ownership of the new one.
    void setOfficialDecision(Decision* officialDecision);

signals:
    void pendingDecisionChanged();
    void officialDecisionChanged(Decision* officialDecision);

protected:
    void updateContestants(::Map<capnp::Text, capnp::Text>::Reader contestantReader);
    void serializeContestants(::Map<capnp::Text, capnp::Text>::Builder contestantBuilder);
    void updateTags(::Map<capnp::Text, capnp::Text>::Reader tagsReader);
    void serializeTags(::Map<capnp::Text, capnp::Text>::Builder tagsBuilder);
};

} } // namespace swv::data

#endif // CONTEST_HPP
