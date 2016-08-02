#include "ContestGeneratorApi.hpp"

#include "Converters.hpp"

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

QJSValue ContestGeneratorApi::getContest() {
    return converter.convert(generator.getContestRequest().send(),
                              [](capnp::Response<ContestGenerator::GetContestResults> response) -> QVariant {
        return {convertListedContest(response.getNextContest())};
    });
}

QJSValue ContestGeneratorApi::getContests(int count) {
    KJ_LOG(DBG, "Requesting contests", count);
    auto request = generator.getContestsRequest();
    request.setCount(count);

    return converter.convert(request.send(),
                              [](capnp::Response<ContestGenerator::GetContestsResults> r) -> QVariant {
        KJ_LOG(DBG, "Got contests", r.getNextContests().size());
        QVariantList contests;
        for (auto contest : r.getNextContests())
            contests.append(convertListedContest(contest));
        return {QVariant(contests)};
    });
}

}
