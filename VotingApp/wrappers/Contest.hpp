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

#include "OwningWrapper.hpp"
#include "Decision.hpp"

#include <contest.capnp.h>

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

namespace swv {

/**
 * @brief The Contest class is a read-only wrapper for the Contest type
 *
 * In addition to exposing the properties of ::UnsignedContest in a QML-accessible form, the Contest implements the
 * concept of the Current Decision for the contest. The current decision is the @ref swv::Decision which should be
 * displayed in the UI as the decision on the contest.
 *
 * The contest may be in one of several states (see @ref State), depending on the chain state and interactions from the
 * user. This state is intended to be set and used by the GUI; it is ignored by all C++ code.
 */
class Contest : public QObject, public ::UnsignedContest::Reader
{
public:
    enum State {
        /// No decision is pending or on chain
        NewPoll,
        /// A decision has not been cast, or has been changed since casting.
        Pending,
        /// A decision has been cast, but has not yet been confirmed as stored on the chain
        Casting,
        /// A decision has been cast and is stored on the chain and has not been changed since
        Cast,
        /// A decision has been cast, but for one of various possible reasons, the decision is no longer counted
        ActionRequired,
        /// The scheduled "end date" of the contest has passed. Decisions can still be cast, but may not 'matter'
        Ended
    };
    Q_ENUM(State)

private:
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QJsonObject tags READ tags CONSTANT)
    Q_PROPERTY(QJsonArray contestants READ contestants CONSTANT)
    Q_PROPERTY(quint64 coin READ getCoin CONSTANT)
    Q_PROPERTY(QDateTime startTime READ startTime CONSTANT)
    Q_PROPERTY(swv::Decision* currentDecision READ currentDecision WRITE setCurrentDecision NOTIFY currentDecisionChanged)
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)

    State m_state;
    OwningWrapper<Decision>* m_currentDecision = nullptr;

public:
    Contest(::UnsignedContest::Reader r, QObject* parent = nullptr);

    // Hexadecimal string containing the ID of the contest
    QString id() const;
    QString name() const {
        return QString::fromStdString(getName());
    }
    QString description() const {
        return QString::fromStdString(getDescription());
    }
    QJsonObject tags() const;
    QJsonArray contestants() const;
    QDateTime startTime() const;

    OwningWrapper<Decision>* currentDecision();
    const OwningWrapper<swv::Decision>* currentDecision() const;

    // Set the current decision. Destroys the old current decision and takes ownership of the new one.
    void setCurrentDecision(OwningWrapper<Decision>* newDecision);
    // Overload of setCurrentDecision. If argument is an OwningWrapper, it casts and calls the other overload.
    // Otherwise, it copies newDecision into a new OwningWrapper<Decision> and calls the other overload.
    void setCurrentDecision(Decision* newDecision);

    State state() const
    {
        return m_state;
    }

public slots:
    void setState(State state)
    {
        if (state == m_state)
            return;

        m_state = state;
        emit stateChanged(state);
    }

signals:
    void currentDecisionChanged();
    void stateChanged(State state);
};

} // namespace swv

#endif // CONTEST_HPP
