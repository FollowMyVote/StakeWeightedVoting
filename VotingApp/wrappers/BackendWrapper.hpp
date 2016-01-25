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

#ifndef BACKENDWRAPPER_HPP
#define BACKENDWRAPPER_HPP

#include <QObject>

#include <backend.capnp.h>

class Promise;
class PromiseConverter;
namespace swv {
class ContestGeneratorWrapper;
class ContestCreatorWrapper;

/**
 * @brief The BackendWrapper class provides a QML-friendly wrapper for the backend API
 *
 * This class makes all of the Cap'nProto RPC interfaces to the backend friendly to QML. It wraps the
 * functions in Q_INVOKABLE proxies and adapts the return values to Qt types and wraps any interfaces
 * the backend returns with other QML-friendly wrappers.
 *
 * All remote calls return a Promise for the result. If the server returns an error on some call, the result promise
 * will be broken, and supplied with the error. This class takes a connected backend client to begin with, and assumes
 * that this client will always be valid. It is the responsibility of the owner of this BackendWrapper to handle
 * disconnections or other failure conditions of the client. The PromiseWrapper passed to this class's constructor will
 * receive any errors from the backend.
 */
class BackendWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ContestCreatorWrapper* contestCreator READ contestCreator CONSTANT);

public:
    BackendWrapper(Backend::Client backend, PromiseConverter& promiseConverter, QObject *parent = 0);
    virtual ~BackendWrapper() noexcept;

    Q_INVOKABLE swv::ContestGeneratorWrapper* getFeedGenerator();
    Q_INVOKABLE swv::ContestGeneratorWrapper* getContestsByCreator(QString creator);
    Q_INVOKABLE swv::ContestGeneratorWrapper* getContestsByCoin(quint64 coinId);

    swv::ContestCreatorWrapper* contestCreator();

private:
    PromiseConverter& promiseConverter;
    Backend::Client backend;
    kj::Own<ContestCreatorWrapper> creator;
};

} // namespace swv
#endif // BACKENDWRAPPER_HPP
