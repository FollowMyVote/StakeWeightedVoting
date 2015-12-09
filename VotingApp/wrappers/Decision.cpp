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

#include "Decision.hpp"

#include <capnp/serialize-packed.h>

#include <kj/common.h>

#include <QDebug>
#include <QJSEngine>
#include <QJSValueIterator>

#define convert(name) \
    auto data = reader().get ## name (); \
    return QByteArray::fromRawData(reinterpret_cast<const char*>(data.begin()), data.size()).toHex()

namespace swv {

Decision::Decision(WrappedType::Builder b, QObject* parent)
    : QObject(parent),
      m_decision(b)
{}

Decision::~Decision() noexcept
{}

QString Decision::id() const
{
    convert(Id);
}

QString Decision::contestId() const
{
    convert(Contest);
}

QVariantMap Decision::opinions() const
{
    auto data = reader().getOpinions();

    QVariantMap results;
    for (::Decision::Opinion::Reader opinion : data)
        results.insert(QString::number(opinion.getContestant()), opinion.getOpinion());
    return results;
}

QVariantList Decision::writeIns() const
{
    auto data = reader().getWriteIns();

    QVariantList results;
    for (::UnsignedContest::Contestant::Reader contestant : data)
        results.append(QVariantMap{{"name", contestant.getName().cStr()},
                                   {"description", contestant.getDescription().cStr()}});
    return results;
}

QVariantMap Decision::canonicalizeOpinions(QVariantMap opinions)
{
    auto itr = opinions.begin();
    while (itr != opinions.end())
        if (itr.value().toInt() == 0)
            itr = opinions.erase(itr);
        else
            ++itr;
    return opinions;
}

void Decision::setOpinions(QVariantMap newOpinions)
{
    canonicalizeOpinions(newOpinions);

    if (newOpinions == opinions())
        return;

    // Now we actually allocate space for and set the opinions
    auto opinionList = m_decision.initOpinions(newOpinions.size());
    unsigned currentIndex = 0;
    while (!newOpinions.empty()) {
        auto builder = opinionList[currentIndex++];
        auto key = newOpinions.firstKey();
        builder.setContestant(key.toInt());
        builder.setOpinion(newOpinions.take(key).toInt());
    }

    emit opinionsChanged();
}

void Decision::setWriteIns(QVariantList newWriteIns)
{
    if (newWriteIns == writeIns())
        return;

    auto writeInList = m_decision.initWriteIns(newWriteIns.size());
    for (::UnsignedContest::Contestant::Builder writeInBuilder : writeInList) {
        auto writeIn = newWriteIns.takeFirst().toMap();
        writeInBuilder.setName(writeIn["name"].toString().toStdString());
        writeInBuilder.setDescription(writeIn["description"].toString().toStdString());
    }

    emit writeInsChanged();
}

} // namespace swv
