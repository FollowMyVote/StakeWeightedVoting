# Copyright 2015 Follow My Vote, Inc.
# This file is part of The Follow My Vote Stake-Weighted Voting Application ("SWV").
#
# SWV is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SWV is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SWV.  If not, see <http://www.gnu.org/licenses/>.

@0xb4ffa96079050068;

interface Generator(Value) {
    # An API to retrieve an 'infinite stream' of values a few at a time. The client can retrieve values as it needs
    # them, rather than fetching a static list all at once. A Value may be any type, including an interface.

    getValues @0 (count :Int32) -> (values :List(Wrapper));
    # Retrieve count more values; may return less than count values if no more values are available

    struct Wrapper {
        value @0 :Value;
    }
}
