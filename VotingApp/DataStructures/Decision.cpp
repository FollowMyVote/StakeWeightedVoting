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
#include "Converters.hpp"
#include "contest.capnp.h"

#include <Utilities.hpp>

#include <capnp/serialize-packed.h>

#include <kj/common.h>

#include <QDebug>
#include <QJSEngine>
#include <QJSValueIterator>

#define convert(name) \
    auto data = reader().get ## name (); \
    return QByteArray::fromRawData(reinterpret_cast<const char*>(data.begin()), data.size()).toHex()

namespace swv { namespace data {

Decision::Decision(::Decision::Reader r, QObject* parent)
    : QObject(parent) {
    updateFields(r);
}

Decision::~Decision() noexcept
{}

void Decision::updateFields(::Decision::Reader r) {
    update_contestId(convertBlob(ReaderPacker(r.getContest()).array()).toHex());
    updateOpinions(r.getOpinions());
    updateWriteIns(r.getWriteIns());
}

void Decision::serialize(::Decision::Builder b) {
    b.setContest(*convertSerialStruct<::ContestId>(m_contestId));
    serializeOpinions(b);
    serializeWriteIns(b);
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

void Decision::updateOpinions(capnp::List<::Decision::Opinion>::Reader opinions) {
    QVariantMap results;
    for (::Decision::Opinion::Reader opinion : opinions)
        results.insert(QString::number(opinion.getContestant()), opinion.getOpinion());
    set_opinions(results);
}

void Decision::updateWriteIns(Map<capnp::Text, capnp::Text>::Reader writeIns) {
    QVariantList results;
    for (auto contestant : writeIns.getEntries())
        results.append(QVariantMap{{"name", contestant.getKey().cStr()},
                                   {"description", contestant.getValue().cStr()}});
    set_writeIns(results);
}

void Decision::serializeOpinions(::Decision::Builder b) {
    auto opinions = canonicalizeOpinions(m_opinions);

    // Now we allocate space for and set the opinions
    auto opinionList = b.initOpinions(opinions.size());
    unsigned currentIndex = 0;
    while (!opinions.empty()) {
        auto builder = opinionList[currentIndex++];
        auto key = opinions.firstKey();
        builder.setContestant(key.toInt());
        builder.setOpinion(opinions.take(key).toInt());
    }
}

void Decision::serializeWriteIns(::Decision::Builder b) {
    auto writeInList = b.initWriteIns().initEntries(m_writeIns.size());
    for (auto writeInBuilder : writeInList) {
        auto writeIn = m_writeIns.takeFirst().toMap();
        writeInBuilder.setKey(writeIn["name"].toString().toStdString());
        writeInBuilder.setValue(writeIn["description"].toString().toStdString());
    }
}

} } // namespace swv::data
