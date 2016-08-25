#include "ContestGeneratorApi.hpp"
#include "ContestResultsApi.hpp"
#include "Converters.hpp"
#include "DataStructures/Contest.hpp"

#include <ids.capnp.h>

#include <kj/debug.h>

namespace swv {

ContestGeneratorApi::ContestGeneratorApi(ContestGenerator::Client generator,
                                         PromiseConverter& converter,
                                         QObject *parent)
    : QObject(parent),
      generator(generator),
      converter(converter)
{}

ContestGeneratorApi::~ContestGeneratorApi() noexcept
{}

QJSValue ContestGeneratorApi::getContests(int count) {
    KJ_LOG(DBG, "Requesting contests", count);
    auto request = generator.getValuesRequest();
    request.setCount(count);
    m_isFetchingContests = true;
    emit isFetchingContestsChanged(true);

    auto pass = [this](auto&& e) {
        m_isFetchingContests = false;
        emit isFetchingContestsChanged(false);
        return kj::mv(e);
    };

    auto contestsPromise = request.send().then([this, count](capnp::Response<ContestGenerator::GetValuesResults> r) {
        if (!r.hasValues() || r.getValues().size() < count) {
            m_isOutOfContests = true;
            emit isOutOfContestsChanged(true);
        }
        m_isFetchingContests = false;
        emit isFetchingContestsChanged(false);
        return kj::mv(r);
    }, pass);

    return converter.convert(kj::mv(contestsPromise),
                             [](capnp::Response<ContestGenerator::GetValuesResults> r) -> QVariant {
        KJ_LOG(DBG, "Got contests", r.getValues().size());
        QVariantList contests;
        for (auto contestWrapper : r.getValues()) {
            auto contest = contestWrapper.getValue();
            auto results = new ContestResultsApi(contest.getContestResults());
            QQmlEngine::setObjectOwnership(results, QQmlEngine::JavaScriptOwnership);
            // TODO: Set engagement notification API (when server defines it)
            contests.append(
                        QVariantMap{
                            {"contestId", QString(convertBlob(ReaderPacker(contest.getContestId()).array()).toHex())},
                            {"votingStake", qint64(contest.getVotingStake())},
                            {"resultsApi", QVariant::fromValue(results)}
                        });
        }
        return {QVariant(contests)};
    });
}

}
