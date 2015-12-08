#ifndef CONTESTGENERATORWRAPPER_HPP
#define CONTESTGENERATORWRAPPER_HPP

#include <contestgenerator.capnp.h>

#include "PromiseConverter.hpp"

#include <Promise.hpp>

namespace swv {

class ContestGeneratorWrapper : public QObject
{
    Q_OBJECT

    ContestGenerator::Client generator;
    PromiseConverter& converter;
public:
    ContestGeneratorWrapper(ContestGenerator::Client generator, PromiseConverter& converter, QObject *parent = 0);
    virtual ~ContestGeneratorWrapper() noexcept;

    Q_INVOKABLE Promise* getContest();
    Q_INVOKABLE Promise* getContests(int count);

signals:

public slots:
};

}

#endif // CONTESTGENERATORWRAPPER_HPP
