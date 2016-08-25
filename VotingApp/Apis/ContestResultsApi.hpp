#ifndef CONTESTRESULTSAPI_HPP
#define CONTESTRESULTSAPI_HPP

#include <contestgenerator.capnp.h>

#include <QObject>
#include <QBarSeries>
#include <QValueAxis>
#include <QBarCategoryAxis>

#include <kj/debug.h>

namespace swv {
namespace data { class Contest; }

class ContestResultsApi : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList contestantResults READ contestantResults NOTIFY resultsChanged)
    Q_PROPERTY(QVariantMap writeInResults READ writeInResults NOTIFY resultsChanged)

    ContestResults::Client resultsApi;

    class ErrorHandler : public kj::TaskSet::ErrorHandler {
    public:
        virtual ~ErrorHandler(){}
        // ErrorHandler interface
        virtual void taskFailed(kj::Exception&& exception) override;
    } errorHandler;
    kj::TaskSet m_tasks;

    class ResultsNotifier : public Notifier<capnp::List<ContestResults::TalliedOpinion>>::Server {
        ContestResultsApi& resultsApi;

    public:
        ResultsNotifier(ContestResultsApi& resultsApi);
        virtual ~ResultsNotifier(){}

    protected:
        virtual ::kj::Promise<void> notify(NotifyContext context) override;
    };

    void updateResults(capnp::List<ContestResults::TalliedOpinion>::Reader talliedOpinions);

    QVariantList m_contestantResults;
    QVariantMap m_writeInResults;

public:
    ContestResultsApi(ContestResults::Client resultsApi);
    virtual ~ContestResultsApi() noexcept;

    QVariantList contestantResults() const { return m_contestantResults; }
    QVariantMap writeInResults() const { return m_writeInResults; }

signals:
    void resultsChanged();
};

} // namespace swv
#endif // CONTESTRESULTSAPI_HPP
