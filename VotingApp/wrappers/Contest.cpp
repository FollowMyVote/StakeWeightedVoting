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

#include "Contest.hpp"
#include "Decision.hpp"

#include <QDebug>

namespace swv {

ContestWrapper::ContestWrapper(UnsignedContest::Reader r, QObject* parent)
    : QObject(parent),
      ::UnsignedContest::Reader(r)
{}

QString ContestWrapper::id() const
{
    auto data = getId();
    return QByteArray::fromRawData(reinterpret_cast<const char*>(data.begin()),
                                   static_cast<signed>(data.size())).toHex();
}

QVariantMap ContestWrapper::tags() const
{
    QVariantMap results;
    auto tagList = getTags().getEntries();
    for (auto tag : tagList)
        results[QString::fromStdString(tag.getKey())] = QString::fromStdString(tag.getValue());
    return results;
}

QVariantList ContestWrapper::contestants() const
{
    QVariantList results;
    auto contestantList = getContestants().getEntries();
    for (auto contestant : contestantList)
        results.append(QVariantMap({{QStringLiteral("name"),
                                     QString::fromStdString(contestant.getKey())},
                                    {QStringLiteral("description"),
                                     QString::fromStdString(contestant.getValue())}}));
    return results;
}

QDateTime ContestWrapper::startTime() const
{
    return QDateTime::fromMSecsSinceEpoch(getStartTime());
}

OwningWrapper<DecisionWrapper>* ContestWrapper::currentDecision() {
    return m_currentDecision;
}

const OwningWrapper<DecisionWrapper>* ContestWrapper::currentDecision() const {
    return m_currentDecision;
}

void ContestWrapper::setCurrentDecision(OwningWrapper<DecisionWrapper>* newDecision)
{
    if (newDecision == nullptr) {
        qDebug() << "Ignoring setCurrentDecision(nullptr)";
        return;
    }

    if (m_currentDecision)
        m_currentDecision->deleteLater();
    newDecision->setParent(this);
    m_currentDecision = newDecision;
    emit currentDecisionChanged();
}

void ContestWrapper::setCurrentDecision(DecisionWrapper* newDecision)
{
    if (newDecision == nullptr) {
        qDebug() << "Ignoring setCurrentDecision(nullptr)";
        return;
    }

    OwningWrapper<DecisionWrapper>* decision = dynamic_cast<OwningWrapper<DecisionWrapper>*>(newDecision);
    if (decision == nullptr)
        decision = new OwningWrapper<DecisionWrapper>(newDecision->reader(), this);
    setCurrentDecision(decision);
}

} // namespace swv
