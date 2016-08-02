#ifndef CONTESTGENERATORWRAPPER_HPP
#define CONTESTGENERATORWRAPPER_HPP

#include <contestgenerator.capnp.h>

#include "PromiseConverter.hpp"

namespace swv {

class ContestGeneratorApi : public QObject {
    Q_OBJECT

    ContestGenerator::Client generator;
    PromiseConverter& converter;
public:
    ContestGeneratorApi(ContestGenerator::Client generator, PromiseConverter& converter, QObject *parent = 0);
    virtual ~ContestGeneratorApi() noexcept;

    Q_INVOKABLE QJSValue getContest();
    Q_INVOKABLE QJSValue getContests(int count);
};

}

#endif // CONTESTGENERATORWRAPPER_HPP
