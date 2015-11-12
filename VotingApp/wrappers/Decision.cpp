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
    return QByteArray::fromRawData((char*)data.begin(), data.size()).toHex()

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

QJSValue Decision::opinions() const
{
    auto data = reader().getOpinions();
    auto engine = qjsEngine(this);
    if (!engine) {
        qDebug() << "Couldn't get the JS Engine! Cannot report opinions.";
        return {};
    }

    QJSValue results = engine->newArray(data.size());
    for (::Decision::Opinion::Reader opinion : data)
        results.setProperty(QString::number(opinion.getContestant()), opinion.getOpinion());
    return results;
}

QJSValue Decision::writeIns() const
{
    auto map = reader().getWriteIns();
    auto engine = qjsEngine(this);
    if (!engine) {
        qDebug() << "Couldn't get the JS Engine! Cannot report write-ins.";
        return {};
    }

    QJSValue results = engine->newArray(map.size());
    int index = 0;
    for (::UnsignedContest::Contestant::Reader contestant : map) {
        auto jsContestant = engine->newObject();
        jsContestant.setProperty("name", contestant.getName().cStr());
        jsContestant.setProperty("description", contestant.getDescription().cStr());
        results.setProperty(index++, jsContestant);
    }
    return results;
}

void Decision::setOpinions(QJSValue newOpinions)
{
    if (newOpinions.strictlyEquals(opinions()))
        return;
    if (!newOpinions.isObject()) {
        qDebug() << "Cannot set opinions because opinions is not an object.";
        return;
    }

    // The only way I know of to determine how many opinions there are is to iterate them and count
    int opinionCount = 0;
    QJSValueIterator itr(newOpinions);
    while (itr.hasNext()) {
        itr.next();
        ++opinionCount;
    }
    itr = newOpinions;

    // Now we actually allocate space for and set the opinions
    auto opinionList = m_decision.initOpinions(opinionCount);
    opinionCount = 0;
    while (itr.hasNext()) {
        itr.next();
        auto builder = opinionList[opinionCount++];
        builder.setContestant(itr.name().toInt());
        builder.setOpinion(itr.value().toInt());
    }

    emit opinionsChanged();
}

void Decision::setWriteIns(QJSValue newWriteIns)
{
    if (newWriteIns.strictlyEquals(this->writeIns()))
        return;
    if (!newWriteIns.isArray()) {
        qDebug() << "Cannot set write-ins because it is not an array.";
        return;
    }

    auto writeInList = m_decision.initWriteIns(newWriteIns.property("length").toInt());
    QJSValueIterator itr(newWriteIns);
    for (::UnsignedContest::Contestant::Builder writeInBuilder : writeInList) {
        itr.next();
        writeInBuilder.setName(itr.value().property("name").toString().toStdString());
        writeInBuilder.setDescription(itr.value().property("description").toString().toStdString());
    }

    emit writeInsChanged();
}

} // namespace swv
