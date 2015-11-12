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

#include <decision.capnp.h>

#include <capnp/message.h>

#include <QObject>
#include <QJSValue>

namespace swv {

/**
 * @brief The Decision class is a read-write wrapper for the Decision type.
 *
 * In addition to exposing the properties of ::Decision in a QML-accessible form, the Decision provides a state
 * property to store the various states a decision may be in. See @ref State
 */
class Decision : public QObject
{
    Q_OBJECT

public:
    using WrappedType = ::Decision;

    enum State {
        // The decision has not been cast, or has been changed since casting. This is the state of an empty
        // (uninitialized) decision
        Pending,
        // The decision has been cast, but has not yet been confirmed as stored on the chain
        Casting,
        // The decision has been cast and is stored on the chain, and has not been changed since
        Cast,
        // The decision has been cast, but the owning balance has been destroyed rendering the decision invalid
        Stale
    };
    Q_ENUM(State)

    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString contestId READ contestId CONSTANT)
    Q_PROPERTY(QJSValue opinions READ opinions WRITE setOpinions NOTIFY opinionsChanged)
    Q_PROPERTY(QJSValue writeIns READ writeIns WRITE setWriteIns NOTIFY writeInsChanged)
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)

    Decision(WrappedType::Builder b, QObject* parent = nullptr);
    ~Decision() noexcept;

    QString id() const;
    QString contestId() const;
    QJSValue opinions() const;
    QJSValue writeIns() const;
    State state() const {
        return m_state;
    }

    ::Decision::Reader reader() const {
        return m_decision.asReader();
    }
    ::Decision::Builder builder() {
        return m_decision;
    }

public slots:
    void setOpinions(QJSValue newOpinions);
    void setWriteIns(QJSValue newWriteIns);
    void setState(State newState) {
        if (newState != m_state)
            m_state = newState;
        emit stateChanged(newState);
    }

signals:
    void balanceIdChanged();
    void opinionsChanged();
    void writeInsChanged();
    void stateChanged(State);

private:
    State m_state = Pending;
    ::Decision::Builder m_decision;
};

} // namespace swv

#endif // DECISION_HPP
