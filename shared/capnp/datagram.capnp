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

const voteMagic :Data = 0x"BA1107";
# Magic number that goes at the beginning of all vote-related datagrams, to identify them as vote-related datagrams

struct Datagram {
# A piece of data stored on the blockchain. Datagrams are stored as belonging to a particular Balance, and the datagram
# index is unique per-balance.

    enum DatagramType {
        decision @0;
        # Marks a datagram as containing a decision. Key will be the publishing balance ID (FC-serialized
        # account_id_type), content will be a Decision struct
        contest @1;
        # Marks a datagram as containing a contest to be created. Key will be a ContestKey; content will be a Contest
        # struct
    }
    struct ContestKey {
    # The schema for the Key field of a contest datagram

        key :union {
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

    index :group {
    # The index is the key which identifies the type and content of the datagram, unique to the balance that stores it
        type @0 :DatagramType;
        key @1 :Data;
    }
    content @2 :Data;
    # The actual data
}
