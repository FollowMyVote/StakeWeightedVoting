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

namespace swv {

Contest::Contest(UnsignedContest::Reader r, QObject* parent)
    : QObject(parent),
      ::UnsignedContest::Reader(r)
{}

QString Contest::id() const
{
    auto data = getId();
    return QByteArray::fromRawData((char*)data.begin(), data.size()).toHex();
}

QJsonObject Contest::tags() const
{
    QJsonObject results;
    auto tagList = getTags();
    for (::UnsignedContest::Tag::Reader tag : tagList)
        results[QString::fromStdString(tag.getKey())] = QString::fromStdString(tag.getValue());
    return results;
}

QJsonArray Contest::contestants() const
{
    QJsonArray results;
    auto contestantList = getContestants();
    for (::UnsignedContest::Contestant::Reader contestant : contestantList)
        results.append(QJsonObject({{QStringLiteral("name"), QString::fromStdString(contestant.getName())},
                                    {QStringLiteral("description"), QString::fromStdString(contestant.getDescription())}}));
    return results;
}

QDateTime Contest::startTime() const
{
    return QDateTime::fromMSecsSinceEpoch(getStartTime());
}

OwningWrapper<Decision>* Contest::currentDecision() {
    return m_currentDecision;
}

const OwningWrapper<Decision>* Contest::currentDecision() const {
    return m_currentDecision;
}

void Contest::setCurrentDecision(OwningWrapper<Decision>* newDecision)
{
    m_currentDecision->deleteLater();
    newDecision->setParent(this);
    m_currentDecision = newDecision;
    emit currentDecisionChanged();
}

} // namespace swv
