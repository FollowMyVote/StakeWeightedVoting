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

#ifndef DECISIONGENERATORAPI_HPP
#define DECISIONGENERATORAPI_HPP

#include <QObject>

#include <decisiongenerator.capnp.h>
#include <generator.capnp.h>

#include "PromiseConverter.hpp"

using DecisionGenerator = Generator<DecisionInfo>;

namespace swv {

class DecisionGeneratorApi : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isOutOfDecisions READ isOutOfDecisions NOTIFY isOutOfDecisionsChanged)
    Q_PROPERTY(bool isFetchingDecisions READ isFetchingDecisions NOTIFY isFetchingDecisionsChanged)

    DecisionGenerator::Client generator;
    PromiseConverter& converter;
    bool m_isOutOfDecisions = false;
    bool m_isFetchingDecisions = false;

public:
    explicit DecisionGeneratorApi(DecisionGenerator::Client generator, PromiseConverter& converter,
                                  QObject *parent = 0);
    virtual ~DecisionGeneratorApi() noexcept {}

    Q_INVOKABLE QJSValue getDecisions(int count);

    bool isOutOfDecisions() const { return m_isOutOfDecisions; }
    bool isFetchingDecisions() const { return m_isFetchingDecisions; }

signals:
    void isOutOfDecisionsChanged(bool isOutOfDecisions);
    void isFetchingDecisionsChanged(bool isFetchingDecisions);
};

} // namespace swv
#endif // DECISIONGENERATORAPI_HPP
