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

Contest::Contest(UnsignedContest::Reader r, QObject* parent)
    : QObject(parent),
      ::UnsignedContest::Reader(r)
{}

QString Contest::id() const
{
    auto data = getId();
    return QByteArray::fromRawData(reinterpret_cast<const char*>(data.begin()),
                                   static_cast<signed>(data.size())).toHex();
}

QVariantMap Contest::tags() const
{
    QVariantMap results;
    auto tagList = getTags();
    for (::UnsignedContest::Tag::Reader tag : tagList)
        results[QString::fromStdString(tag.getKey())] = QString::fromStdString(tag.getValue());
    return results;
}

QVariantList Contest::contestants() const
{
    QVariantList results;
    auto contestantList = getContestants();
    for (::UnsignedContest::Contestant::Reader contestant : contestantList)
        results.append(QVariantMap({{QStringLiteral("name"),
                                     QString::fromStdString(contestant.getName())},
                                    {QStringLiteral("description"),
                                     QString::fromStdString(contestant.getDescription())}}));
    return results;
}

QDateTime Contest::startTime() const
{
    return QDateTime::fromMSecsSinceEpoch(static_cast<signed>(getStartTime()));
}

OwningWrapper<Decision>* Contest::currentDecision() {
    return m_currentDecision;
}

const OwningWrapper<Decision>* Contest::currentDecision() const {
    return m_currentDecision;
}

void Contest::setCurrentDecision(OwningWrapper<Decision>* newDecision)
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

void Contest::setCurrentDecision(Decision* newDecision)
{
    if (newDecision == nullptr) {
        qDebug() << "Ignoring setCurrentDecision(nullptr)";
        return;
    }

    OwningWrapper<Decision>* decision = dynamic_cast<OwningWrapper<Decision>*>(newDecision);
    if (decision == nullptr)
        decision = new OwningWrapper<Decision>(newDecision->reader(), this);
    setCurrentDecision(decision);
}

} // namespace swv
