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

#ifndef CONTESTGENERATORWRAPPER_HPP
#define CONTESTGENERATORWRAPPER_HPP

#include <contestgenerator.capnp.h>
#include <generator.capnp.h>
#include <contest.capnp.h>

#include "PromiseConverter.hpp"

#include <QQmlEnumClassHelper.h>

using ContestGenerator = Generator<::ContestInfo>;

namespace swv {

QML_ENUM_CLASS(Engagement,
               expanded = static_cast<int>(::EngagementType::EXPANDED),
               voted = static_cast<int>(::EngagementType::VOTED),
               liked = static_cast<int>(::EngagementType::LIKED))

class ContestGeneratorApi : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isOutOfContests READ isOutOfContests NOTIFY isOutOfContestsChanged)
    Q_PROPERTY(bool isFetchingContests READ isFetchingContests NOTIFY isFetchingContestsChanged)

    ContestGenerator::Client generator;
    PromiseConverter& converter;
    bool m_isOutOfContests = false;
    bool m_isFetchingContests = false;

public:

    ContestGeneratorApi(ContestGenerator::Client generator, PromiseConverter& converter, QObject *parent = 0);
    virtual ~ContestGeneratorApi() noexcept;

    Q_INVOKABLE QJSValue getContests(int count);

    Q_INVOKABLE void takeOwnership(QObject* obj) {
        if (obj) obj->setParent(this);
    }

    bool isOutOfContests() const { return m_isOutOfContests; }
    bool isFetchingContests() const { return m_isFetchingContests; }

signals:
    void isOutOfContestsChanged(bool isOutOfContests);
    void isFetchingContestsChanged(bool isFetchingContests);
};

}

#endif // CONTESTGENERATORWRAPPER_HPP
