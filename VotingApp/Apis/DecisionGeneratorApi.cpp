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

#include "DecisionGeneratorApi.hpp"

#include <Converters.hpp>

namespace swv {

DecisionGeneratorApi::DecisionGeneratorApi(DecisionGenerator::Client generator, PromiseConverter& converter,
                                           QObject *parent)
    : QObject(parent),
      generator(generator),
      converter(converter) {}

QJSValue DecisionGeneratorApi::getDecisions(int count) {
    if (count > std::numeric_limits<uint8_t>::max())
        // Fail hard for this; the GUI should guarantee count is valid
        qFatal("Application bug: count must fit in a uint8_t");

    KJ_LOG(DBG, "Requesting decisions", count);
    auto request = generator.getValuesRequest();
    request.setCount(count);
    m_isFetchingDecisions = true;
    emit isFetchingDecisionsChanged(true);

    auto pass = [this](auto&& e) {
        m_isFetchingDecisions = false;
        emit isFetchingDecisionsChanged(false);
        return kj::mv(e);
    };

    auto contestsPromise = request.send().then([this, count](capnp::Response<DecisionGenerator::GetValuesResults> r) {
        if (!r.hasValues() || r.getValues().size() < count) {
            m_isOutOfDecisions = true;
            emit isOutOfDecisionsChanged(true);
        }
        m_isFetchingDecisions = false;
        emit isFetchingDecisionsChanged(false);
        return kj::mv(r);
    }, pass);

    return converter.convert(kj::mv(contestsPromise),
                             [](capnp::Response<DecisionGenerator::GetValuesResults> r) -> QVariant {
        KJ_LOG(DBG, "Got decisions", r.getValues().size());
        QVariantList decisions;
        for (auto decisionWrapper : r.getValues()) {
            auto decision = decisionWrapper.getValue();
            decisions.append(
                        QVariantMap{
                            {"decisionId", QString(convertBlob(ReaderPacker(decision.getId()).array()).toHex())},
                            {"counted", decision.getCounted()}
                        });
        }
        return QVariant(decisions);
    });
}

} // namespace swv
