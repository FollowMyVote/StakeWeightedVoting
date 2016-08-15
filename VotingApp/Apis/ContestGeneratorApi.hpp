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
    Q_PROPERTY(bool isOutOfContests READ isOutOfContests NOTIFY isOutOfContestsChanged)
    Q_PROPERTY(bool isFetchingContests READ isFetchingContests NOTIFY isFetchingContestsChanged)

    ContestGenerator::Client generator;
    PromiseConverter& converter;
    bool m_isOutOfContests = false;
    bool m_isFetchingContests = false;

public:
    ContestGeneratorApi(ContestGenerator::Client generator, PromiseConverter& converter, QObject *parent = 0);
    virtual ~ContestGeneratorApi() noexcept;

    Q_INVOKABLE QJSValue getContest();
    Q_INVOKABLE QJSValue getContests(int count);

    Q_INVOKABLE void logEngagement(QString contestId, Engagement::Type engagementType);

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
