#ifndef CONTESTRESULTSAPI_HPP
#define CONTESTRESULTSAPI_HPP

#include <backend.capnp.h>
#include <purchase.capnp.h>

#include <QObject>
#include <QBarSeries>

#include <kj/debug.h>

namespace swv {
using QtCharts::QBarSeries;

namespace data { class Contest; }

class ContestResultsApi : public QObject {
    Q_OBJECT
    Q_PROPERTY(QBarSeries* results READ results WRITE setResults NOTIFY resultsChanged)

    Backend::ContestResults::Client resultsApi;
    const data::Contest& contest;

    QBarSeries* m_results = new QBarSeries(this);

    class ErrorHandler : public kj::TaskSet::ErrorHandler {
    public:
        virtual ~ErrorHandler(){}
        // ErrorHandler interface
        virtual void taskFailed(kj::Exception&& exception) override;
    } errorHandler;
    kj::TaskSet m_tasks;

    class ResultsNotifier : public Notifier<capnp::List<Backend::ContestResults::TalliedOpinion>>::Server {
        ContestResultsApi& resultsApi;

    public:
        ResultsNotifier(ContestResultsApi& resultsApi);
        virtual ~ResultsNotifier(){}

    protected:
        virtual ::kj::Promise<void> notify(NotifyContext context) override;
    };

    void updateBarSeries(capnp::List<Backend::ContestResults::TalliedOpinion>::Reader talliedOpinions);

public:
    ContestResultsApi(Backend::ContestResults::Client resultsApi, const data::Contest& contest);
    virtual ~ContestResultsApi() noexcept;

    QBarSeries* results() { return m_results; }
    void setResults(QBarSeries* results);

signals:
    void resultsChanged(const QBarSeries* results);
};

} // namespace swv
#endif // CONTESTRESULTSAPI_HPP
