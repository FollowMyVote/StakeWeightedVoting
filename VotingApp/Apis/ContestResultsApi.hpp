#ifndef CONTESTRESULTSAPI_HPP
#define CONTESTRESULTSAPI_HPP

#include <backend.capnp.h>
#include <purchase.capnp.h>

#include <QObject>
#include <QBarSeries>

#include <kj/debug.h>

namespace swv {
using QtCharts::QBarSeries;

class ContestResultsApi : public QObject {
    Q_OBJECT
    Q_PROPERTY(const QBarSeries* results READ results NOTIFY resultsChanged)

    Backend::ContestResults::Client resultsApi;

    QBarSeries m_results;

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
    ContestResultsApi(Backend::ContestResults::Client resultsApi);
    virtual ~ContestResultsApi() noexcept;

    const QBarSeries* results() const {
        return &m_results;
    }

signals:
    void resultsChanged(const QBarSeries* results);
};

} // namespace swv
#endif // CONTESTRESULTSAPI_HPP
