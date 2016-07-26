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

#include "PromiseConverter.hpp"

namespace swv {

PromiseConverter::PromiseConverter(kj::TaskSet& tasks, QObject* promiseParent, QObject* parent)
    : QObject(parent),
      promiseParent(promiseParent),
      tasks(tasks)
{}

QJSValue PromiseConverter::convert(kj::Promise<void> promise)
{
    auto convertedPromise = kj::heap<QmlPromise>(promiseParent);

    auto responsePromise = promise.then(
                [convertedPromise = convertedPromise.get()] {
        convertedPromise->resolve();
    }, [convertedPromise = convertedPromise.get()](kj::Exception&& exception) {
        convertedPromise->reject({QString::fromStdString(exception.getDescription())});
        KJ_LOG(WARNING, "Exception in PromiseConverter", exception);
    });

    QJSValue result = *convertedPromise;
    tasks.add(responsePromise.attach(kj::mv(convertedPromise)));

    return result;
}

} // namespace swv
