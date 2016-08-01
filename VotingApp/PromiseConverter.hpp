/*
 * Copyright 2015 Follow My Vote, Inc.
 * This file is part of The Follow My Vote Stake-Weighted Voting Application ("SWV").
 *
 * SWV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SWV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SWV.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PROMISEWRAPPER_HPP
#define PROMISEWRAPPER_HPP

#include "qppromise.h"

#include <kj/async.h>
#include <kj/debug.h>

#include <QObject>
#include <QQmlEngine>

namespace swv {

/**
 * @brief The PromiseConverter class converts kj::Promise objects to QML-friendly Promise objects.
 *
 * The Promise objects created by PromiseConverter are parented to the PromiseConverter initially, as they must not be
 * deleted prior to resolution; however, once the promise resolves it is set to have JavaScript Ownership so it can be
 * garbage collected by the QML runtime. The Promise remains parented to the converter, so if the QML runtime never
 * garbage collects the Promise, it will be deleted when the PromiseWrapper is deleted.
 *
 * If the promise is broken, the converted Promise will also break. If the converted Promise does not have a rejection
 * handler, the exception will be propagated to the TaskSet provided to the constructor. Either way, the returned
 * Promise will be set to Rejected.
 */
class PromiseConverter : public QObject
{
    Q_OBJECT
    QObject* promiseParent;
public:
    /// @param promiseParent An object known to the QML engine, to be set as the parent of the created Promises
    explicit PromiseConverter(kj::TaskSet& tasks, QObject* promiseParent, QObject *parent = 0);
    virtual ~PromiseConverter() noexcept {}

    /**
     * @brief Convert a kj promise to a QML-friendly promise
     * @param promise The promise to convert
     * @param TConverter A callable taking a T as an argument and returning a QVariant
     * @return A Promise which fulfills or breaks with the provided promise
     * @tparam PromisedType The type the kj promise resolves to
     *
     * Takes promise, converts it to a Promise*, and saves promise to ensure it is evaluated. If promise is broken, the
     * returned promise will be broken as well.
     *
     * The TConverter is necessary because the T returned by promise must be converted to a list of QVariants in order
     * for it to be meaningful to QML. TConverter effects this conversion.
     */
    template<typename PromisedType, typename Func>
    QJSValue convert(kj::Promise<PromisedType> promise, Func TConverter);
    QJSValue convert(kj::Promise<void> promise);

    /// @brief Take a promise and ensure it completes or report the failure, but do not convert it
    void adopt(kj::Promise<void>&& promise) {
        tasks.add(kj::mv(promise));
    }

private:
    kj::TaskSet& tasks;
};

template<typename T, typename Func>
QJSValue PromiseConverter::convert(kj::Promise<T> promise, Func TConverter) {
    auto convertedPromise = kj::heap<QPPromise>(promiseParent);

    auto responsePromise = promise.then(
        [convertedPromise = convertedPromise.get(), TConverter](T&& results) {
            convertedPromise->resolve(TConverter(kj::mv(results)));
        }, [convertedPromise = convertedPromise.get()](kj::Exception&& exception) {
            convertedPromise->reject({QString::fromStdString(exception.getDescription())});
            KJ_LOG(WARNING, "Exception in PromiseConverter", exception);
        });
    tasks.add(responsePromise.attach(kj::mv(convertedPromise)));

    return *convertedPromise;
}

} // namespace swv
#endif // PROMISEWRAPPER_HPP
