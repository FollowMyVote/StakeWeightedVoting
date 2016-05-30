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
#include <QVariantMap>

namespace swv {

/**
 * @brief The DecisionWrapper class is a read-write wrapper for the Decision type.
 */
class DecisionWrapper : public QObject
{
    Q_OBJECT

public:
    using WrappedType = ::Decision;

    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString contestId READ contestId CONSTANT)
    Q_PROPERTY(QVariantMap opinions READ opinions WRITE setOpinions NOTIFY opinionsChanged)
    Q_PROPERTY(QVariantList writeIns READ writeIns WRITE setWriteIns NOTIFY writeInsChanged)

    DecisionWrapper(WrappedType::Builder b, QObject* parent = nullptr);
    ~DecisionWrapper() noexcept;

    QString id() const;
    QString contestId() const;
    QVariantMap opinions() const;
    QVariantList writeIns() const;

    ::Decision::Reader reader() const {
        return m_decision.asReader();
    }
    ::Decision::Builder builder() {
        return m_decision;
    }

    /// @brief Compare two decisions. Decisions are equal if they apply to the same contest and have the same opinions
    /// and write-ins. The IDs are not relevant to equality.
    bool operator== (const DecisionWrapper& other) {
        return contestId() == other.contestId() &&
                opinions() == other.opinions() &&
                writeIns() == other.writeIns();
    }
    bool operator!= (const DecisionWrapper& other) {
        return !(*this == other);
    }

public slots:
    void setOpinions(QVariantMap newOpinions);
    void setWriteIns(QVariantList newWriteIns);

signals:
    void balanceIdChanged();
    void opinionsChanged();
    void writeInsChanged();

private:
    /// Remove all opinions of zero
    QVariantMap canonicalizeOpinions(QVariantMap opinions);

    ::Decision::Builder m_decision;
};

} // namespace swv

#endif // DECISION_HPP
