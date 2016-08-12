#ifndef CONTESTRESULTSAPI_HPP
#define CONTESTRESULTSAPI_HPP

#include <backend.capnp.h>
#include <purchase.capnp.h>

#include <QObject>
#include <QBarSeries>
#include <QValueAxis>
#include <QBarCategoryAxis>

#include <kj/debug.h>

namespace swv {
namespace data { class Contest; }

class ContestResultsApi : public QObject {
    Q_OBJECT
    Q_PROPERTY(QtCharts::QBarSeries* results READ results WRITE setResults NOTIFY resultsChanged)
    Q_PROPERTY(QtCharts::QBarCategoryAxis* xAxis READ xAxis CONSTANT)
    Q_PROPERTY(QtCharts::QValueAxis* yAxis READ yAxis CONSTANT)

    Backend::ContestResults::Client resultsApi;
    const data::Contest& contest;

    QtCharts::QBarSeries* m_results = new QtCharts::QBarSeries(this);
    QtCharts::QBarCategoryAxis* m_xAxis = new QtCharts::QBarCategoryAxis(this);
    QtCharts::QValueAxis* m_yAxis = new QtCharts::QValueAxis(this);

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

    QtCharts::QBarSeries* results() { return m_results; }
    void setResults(QtCharts::QBarSeries* results);

    QtCharts::QBarCategoryAxis* xAxis() const { return m_xAxis; }
    QtCharts::QValueAxis* yAxis() const { return m_yAxis; }

signals:
    void resultsChanged(const QtCharts::QBarSeries* results);
};

} // namespace swv
#endif // CONTESTRESULTSAPI_HPP
