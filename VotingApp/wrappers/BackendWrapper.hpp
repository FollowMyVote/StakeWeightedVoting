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

/**
 * @brief The BackendWrapper class provides a QML-friendly wrapper for the RPC backend
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
public:
    BackendWrapper(Backend::Client backend, PromiseConverter& promiseConverter, QObject *parent = 0);
    virtual ~BackendWrapper() noexcept {}

    Q_INVOKABLE Promise* increment(quint8 num);

    /**
     * @section Contest Feed
     *
     * These methods manage the contest feed. The @ref getContest and @ref getContests methods each statefully fetch
     * the next 1 or N contests from the server for the feed.
     *
     * If it is desired to throw away the state on which contests have been fetched and begin retrieving the list again
     * from scratch, @ref refreshContests should be called. Subsequent calls to @ref getContest and @ref getContests
     * will return contests as though none had already been fetched, so they may return a few new contests which just
     * launched, then they may return the same sequence of contests they returned initially.
     * @{
     */
    /**
     * @brief Get a contest for the feed from the backend
     *
     * The returned promise resolves to an object of the form:
     * {
     *     "contestId" : <contest ID>,
     *     "votingStake" : <total stake which has voted on this contest>,
     *     "tracksLiveResults" : <whether the server has live results for this contest or not>
     * }
     */
    Q_INVOKABLE Promise* getContest();
    /**
     * @brief Get count contests for the feed from the backend
     *
     * The returned promise resolves to an array of objects of the form returned by @ref getFeedContest
     */
    Q_INVOKABLE Promise* getContests(int count);
    /**
     * @brief Refresh contest list so @ref getContest and @ref getContests lose state and return contests from the
     * beginning of the feed again
     */
    Q_SLOT void refreshContests();
    /// @}

private:
    PromiseConverter& promiseConverter;
    Backend::Client backend;
    kj::ForkedPromise<ContestGenerator::Client> generatorPromise;
};

} // namespace swv
#endif // BACKENDWRAPPER_HPP
