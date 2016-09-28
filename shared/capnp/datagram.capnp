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

@0x8ed11a20887e5444;

using ContestId = import "ids.capnp".ContestId;

const voteMagic :Data = 0x"BA1107";
# Magic number that goes at the beginning of all vote-related datagrams, to identify them as vote-related datagrams

struct Datagram {
# A piece of data stored on the blockchain. Datagrams are stored as belonging to a particular Balance, and the datagram
# key is unique per-balance.

    struct DecisionKey {
    # The schema for the Key field of a decision datagram
        contestId @0 :ContestId;
    }
    struct ContestKey {
    # The schema for the Key field of a contest datagram
        creator :union {
            anonymous @0 :Void;
            # No signature; creator is anonymous
            signature :group {
                id @1 :Data;
                # FC-serialized graphene::chain::account_id_type, the creator's account ID
                signature @2 :Data;
                # FC-serialized fc::ecc::compact_signature, signature from the creator's memo key
            }
        }
    }
    struct DatagramKey {
        key :union {
            decisionKey @0 :DecisionKey;
            contestKey @1 :ContestKey;
        }
    }

    key @0 :DatagramKey;
    content @1 :Data;
    # The actual data
}
