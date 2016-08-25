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
