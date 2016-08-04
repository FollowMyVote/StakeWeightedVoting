#ifndef CONTESTGENERATORWRAPPER_HPP
#define CONTESTGENERATORWRAPPER_HPP

#include <contestgenerator.capnp.h>

#include "PromiseConverter.hpp"

#include <QQmlEnumClassHelper.h>

namespace swv {

QML_ENUM_CLASS(Engagement,
               expanded = (int)ContestGenerator::EngagementType::EXPANDED,
               voted = (int)ContestGenerator::EngagementType::VOTED,
               liked = (int)ContestGenerator::EngagementType::LIKED)

class ContestGeneratorApi : public QObject {
    Q_OBJECT

    ContestGenerator::Client generator;
    PromiseConverter& converter;
public:
    ContestGeneratorApi(ContestGenerator::Client generator, PromiseConverter& converter, QObject *parent = 0);
    virtual ~ContestGeneratorApi() noexcept;

    Q_INVOKABLE QJSValue getContest();
    Q_INVOKABLE QJSValue getContests(int count);

    Q_INVOKABLE void logEngagement(QString contestId, Engagement::Type engagementType);
};

}

#endif // CONTESTGENERATORWRAPPER_HPP
