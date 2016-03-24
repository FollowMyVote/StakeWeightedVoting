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
#ifndef BACKENDPLUGIN_HPP
#define BACKENDPLUGIN_HPP

#include <graphene/app/plugin.hpp>

#include <kj/async-io.h>

namespace swv {
class TwoPartyServer;

class BackendPlugin : public graphene::app::plugin
{
    kj::Own<TwoPartyServer> server;
    kj::AsyncIoContext asyncIo = kj::setupAsyncIo();
    kj::Promise<void> serverPromise = kj::READY_NOW;
    uint16_t serverPort = 17073;

public:
    BackendPlugin();
    virtual ~BackendPlugin() noexcept;

    // abstract_plugin interface
    virtual std::string plugin_name() const override;
    virtual void plugin_initialize(const boost::program_options::variables_map& options) override;
    virtual void plugin_startup() override;
    virtual void plugin_shutdown() override;
    virtual void plugin_set_program_options(boost::program_options::options_description& command_line_options,
                                            boost::program_options::options_description& config_file_options) override;
};

} // namespace swv

#endif // BACKENDPLUGIN_HPP
