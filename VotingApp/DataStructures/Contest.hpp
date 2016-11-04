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
#include <QQmlEnumClassHelper.h>

#include <QObject>
#include <QDateTime>

namespace swv { namespace data {

/**
 * @brief The Contest class is a QML-friendly presentation of the data in a capnp UnsignedContest
 *
 * In addition to exposing the properties of ::UnsignedContest in a QML-accessible form, Contest provides storage for
 * two data::Decision objects: the Pending Decision and the Official Decision.
 *
 * The Pending Decision should be used by the GUI to store the user's currently selected opinions which have not yet
 * been committed to the blockchain. This property should always be populated with a Decision object, even if that
 * decision has no opinions, or just mirrors the Official Decision.
 *
 * The Official Decision should contain the decision currently on record (i.e. the newest one on the blockchain) for
 * the current user. If the user has not broadcast any decision to the blockchain, the Official Decision should be
 * nullptr.
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
    /// The pending decision is the decision the user has input, but not yet committed to the blockchain
    /// Note that this property is provided merely as storage and is not used or updated by Contest.
    Q_PROPERTY(swv::data::Decision* pendingDecision READ pendingDecision WRITE setPendingDecision NOTIFY pendingDecisionChanged)
    /// The official decision is the on-chain decision of record
    /// Note that this property is provided merely as storage and is not used or updated by Contest.
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

    /**
     * @brief Get the name for the specified candidate
     * @param candidateId The ID of the candidate to get a name for
     * @param decision The decision from which the candidate ID was taken
     * @return Name of the candidate having the provided ID
     *
     * On its face, it seems like getting the name for a candidate would require only the contest (and particularly,
     * the list of contestants), but this isn't actually so because decisions may specify write-in candidates. Thus it
     * is necessary to know the decision in order to get the candidate name, in case the candidate is a write-in and
     * not a well-known contestant.
     */
    Q_INVOKABLE QString getCandidateName(int candidateId, swv::data::Decision* decision);

    /**
     * @brief Get a simplified format of the contest ID
     *
     * Instead of a long-form contest ID, such as 100210010108, returns simply 8
     * @note This may not be applicable on all blockchains, in which case this simply returns the unmodified ID
     */
    Q_INVOKABLE QString simpleId() const;

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

} // namespace swv::data

QML_ENUM_CLASS(ContestDecisionStatus, NoDecision, PendingDecision, OfficialDecision)

} // namespace swv

#endif // CONTEST_HPP
