#include "ContestGeneratorWrapper.hpp"

#include "Converters.hpp"

#include <kj/debug.h>

namespace swv {

ContestGeneratorWrapper::ContestGeneratorWrapper(ContestGenerator::Client generator,
                                                 PromiseConverter& converter,
                                                 QObject *parent)
    : QObject(parent),
      generator(generator),
      converter(converter)
{}

ContestGeneratorWrapper::~ContestGeneratorWrapper() noexcept
{}

Promise* ContestGeneratorWrapper::getContest()
{
    return converter.convert(generator.getContestRequest().send(),
                              [](capnp::Response<ContestGenerator::GetContestResults> response) -> QVariantList {
        return {convertListedContest(response.getNextContest())};
    });
}

Promise* ContestGeneratorWrapper::getContests(int count)
{
    KJ_LOG(DBG, "Requesting contests", count);
    auto request = generator.getContestsRequest();
    request.setCount(count);

    return converter.convert(request.send(),
                              [](capnp::Response<ContestGenerator::GetContestsResults> r) -> QVariantList {
        KJ_LOG(DBG, "Got contests", r.getNextContests().size());
        QVariantList contests;
        for (auto contest : r.getNextContests())
            contests.append(convertListedContest(contest));
        return {QVariant(contests)};
    });
}

}
