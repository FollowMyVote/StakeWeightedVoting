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
#include <QDateTime>

namespace swv {

/**
 * @brief The ContestWrapper class is a read-only wrapper for the Contest type
 *
 * In addition to exposing the properties of ::UnsignedContest in a QML-accessible form, the Contest implements the
 * concept of the Current Decision for the contest. The current decision is the @ref swv::Decision which should be
 * displayed in the UI as the decision on the contest.
 */
class ContestWrapper : public QObject, public ::Contest::Reader
{
private:
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QVariantMap tags READ tags CONSTANT)
    Q_PROPERTY(QVariantList contestants READ contestants CONSTANT)
    Q_PROPERTY(quint64 coin READ getCoin CONSTANT)
    Q_PROPERTY(QDateTime startTime READ startTime CONSTANT)
    Q_PROPERTY(swv::DecisionWrapper* currentDecision READ currentDecision WRITE setCurrentDecision NOTIFY currentDecisionChanged)

    OwningWrapper<DecisionWrapper>* m_currentDecision = nullptr;

public:
    ContestWrapper(::Contest::Reader r, QObject* parent = nullptr);

    // Hexadecimal string containing the ID of the contest
    QString id() const;
    QString name() const {
        return QString::fromStdString(getName());
    }
    QString description() const {
        return QString::fromStdString(getDescription());
    }
    QVariantMap tags() const;
    QVariantList contestants() const;
    QDateTime startTime() const;

    OwningWrapper<DecisionWrapper>* currentDecision();
    const OwningWrapper<swv::DecisionWrapper>* currentDecision() const;

    // Set the current decision. Destroys the old current decision and takes ownership of the new one.
    void setCurrentDecision(OwningWrapper<DecisionWrapper>* newDecision);
    // Overload of setCurrentDecision. If argument is an OwningWrapper, it casts and calls the other overload.
    // Otherwise, it copies newDecision into a new OwningWrapper<Decision> and calls the other overload.
    void setCurrentDecision(DecisionWrapper* newDecision);

signals:
    void currentDecisionChanged();
};

} // namespace swv

#endif // CONTEST_HPP
