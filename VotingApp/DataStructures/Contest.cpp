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

Contest::Contest(QString id, QObject* parent)
    : QObject(parent),
      m_id(id) {}

Contest::~Contest() { qDebug() << "Destroyed Contest" << m_name; }

void Contest::updateFields(::Contest::Reader r) {
    update_coin(r.getCoin());
    update_description(QString::fromStdString(r.getDescription()));
    update_name(QString::fromStdString(r.getName()));
    update_startTime(QDateTime::fromMSecsSinceEpoch(r.getStartTime()));
    updateContestants(r.getContestants());
    updateTags(r.getTags());
}

void Contest::serialize(::Contest::Builder b) {
    b.setCoin(m_coin);
    b.setDescription(m_description.toStdString());
    b.setName(m_name.toStdString());
    b.setStartTime(m_startTime.toMSecsSinceEpoch());
    serializeContestants(b.initContestants());
    serializeTags(b.initTags());
}

void Contest::setPendingDecision(Decision* newDecision) {
    if (newDecision == m_pendingDecision)
        return;

    if (m_pendingDecision)
        m_pendingDecision->deleteLater();
    if (newDecision)
        newDecision->setParent(this);

    m_pendingDecision = newDecision;
    emit pendingDecisionChanged();
}

void Contest::setOfficialDecision(Decision* officialDecision) {
    if (m_officialDecision == officialDecision)
        return;

    if (m_officialDecision)
        m_officialDecision->deleteLater();
    if (officialDecision)
        officialDecision->setParent(this);

    m_officialDecision = officialDecision;
    emit officialDecisionChanged(officialDecision);
}

void Contest::updateContestants(::Map<capnp::Text, capnp::Text>::Reader contestantReader) {
    for (auto contestant : contestantReader.getEntries())
        m_contestants.append(QVariantMap({{QStringLiteral("name"),
                                           QString::fromStdString(contestant.getKey())},
                                          {QStringLiteral("description"),
                                           QString::fromStdString(contestant.getValue())}}));
}

void Contest::serializeContestants(::Map<capnp::Text, capnp::Text>::Builder contestantBuilder) {
    auto contestants = contestantBuilder.initEntries(m_contestants.size());
    for (unsigned i = 0; i < contestants.size(); ++i) {
        contestants[i].setKey(m_contestants[i].toMap()["name"].toString().toStdString());
        contestants[i].setValue(m_contestants[i].toMap()["description"].toString().toStdString());
    }
}

void Contest::updateTags(::Map<capnp::Text, capnp::Text>::Reader tagsReader) {
    for (auto tag : tagsReader.getEntries())
        m_tags.insert(QString::fromStdString(tag.getKey()), QString::fromStdString(tag.getValue()));
}

void Contest::serializeTags(::Map<capnp::Text, capnp::Text>::Builder tagsBuilder) {
    auto tags = tagsBuilder.initEntries(m_tags.size());
    unsigned index = 0;
    for (const auto& tagName : m_tags.keys()) {
        auto tagBuilder = tags[index++];
        tagBuilder.setKey(tagName.toStdString());
        tagBuilder.setValue(m_tags[tagName].toString().toStdString());
    }
}

} } // namespace swv::data
