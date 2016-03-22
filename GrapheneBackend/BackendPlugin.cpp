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
#include "BackendPlugin.hpp"

namespace swv {

BackendPlugin::BackendPlugin()
{

}

std::string BackendPlugin::plugin_name() const {
    return "Follow My Vote Backend";
}

void BackendPlugin::plugin_initialize(const boost::program_options::variables_map& options) {
}

void BackendPlugin::plugin_startup() {
}

void BackendPlugin::plugin_shutdown() {
}

void BackendPlugin::plugin_set_program_options(boost::program_options::options_description& command_line_options,
                                                    boost::program_options::options_description& config_file_options) {
    namespace bpo = boost::program_options;
    command_line_options.add_options()("port,p", bpo::value<uint16_t>()->default_value(17073),
                                       "The port for the server to listen on");
    config_file_options.add_options()("port,p", bpo::value<uint16_t>()->default_value(17073),
                                      "The port for the server to listen on");
}

} // namespace swv
