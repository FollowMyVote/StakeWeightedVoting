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

#include <kj/async.h>
#include <kj/debug.h>

#include <QObject>
#include <QQmlEngine>

#include "Promise.hpp"

/**
 * @brief The PromiseConverter class converts kj::Promise objects to QML-friendly Promise objects.
 *
 * The Promise objects created by PromiseConverter are parented to the PromiseConverter initially, as they must not be
 * deleted prior to resolution; however, once the promise resolves it is set to have JavaScript Ownership so it can be
 * garbage collected by the QML runtime. The Promise remains parented to the converter, so if the QML runtime never
 * garbage collects the Promise, it will be deleted when the PromiseWrapper is deleted.
 */
class PromiseConverter : public QObject
{
    Q_OBJECT
public:
    explicit PromiseConverter(kj::TaskSet& tasks, QObject *parent = 0);
    virtual ~PromiseConverter() noexcept {}

    /**
     * @brief Convert a kj promise to a QML-friendly promise
     * @param promise The promise to convert
     * @param TConverter A callable taking a T as an argument and returning a QVariantList
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
    Promise* convert(kj::Promise<PromisedType> promise, Func TConverter);
    Promise* convert(kj::Promise<void> promise);

private:
    kj::TaskSet& tasks;
};

template<typename T, typename Func>
Promise* PromiseConverter::convert(kj::Promise<T> promise, Func TConverter)
{
    auto result = new Promise(this);

    auto responsePromise = promise.then(
        [result, TConverter](T&& results) {
            result->resolve(TConverter(kj::mv(results)));
            QQmlEngine::setObjectOwnership(result, QQmlEngine::JavaScriptOwnership);
        }, [result](kj::Exception&& exception) {
            result->reject({QString::fromStdString(exception.getDescription())});
            QQmlEngine::setObjectOwnership(result, QQmlEngine::JavaScriptOwnership);
            throw exception;
        });
    tasks.add(kj::mv(responsePromise));

    return result;
}

#endif // PROMISEWRAPPER_HPP
