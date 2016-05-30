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

#include <QDebug>

namespace swv { namespace data {

Contest::Contest(QString id, ::Contest::Reader r, QObject* parent)
    : QObject(parent),
      m_id(id) {
    m_name = QString::fromStdString(r.getName());
    m_description = QString::fromStdString(r.getDescription());
    for (auto tag : r.getTags().getEntries())
        m_tags.insert(QString::fromStdString(tag.getKey()), QString::fromStdString(tag.getValue()));
    for (auto contestant : r.getContestants().getEntries())
        m_contestants.append(QVariantMap({{QStringLiteral("name"),
                                           QString::fromStdString(contestant.getKey())},
                                          {QStringLiteral("description"),
                                           QString::fromStdString(contestant.getValue())}}));
    m_coin = r.getCoin();
    m_startTime = QDateTime::fromMSecsSinceEpoch(r.getStartTime());
}

void Contest::setCurrentDecision(OwningWrapper<DecisionWrapper>* newDecision)
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

void Contest::setCurrentDecision(DecisionWrapper* newDecision)
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

} } // namespace swv::data
