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

@0x8dbd6b13e9c96a0e;

using Map = import "map.capnp".Map;
using Purchase = import "purchase.capnp".Purchase;
using ContestOptions = import "contest.capnp".Contest;
using ContestCreatorSignature = import "datagram.capnp".Datagram.ContestKey;

interface ContestCreator {
# The ContestCreator interface handles the protocol for creating a contest, including getting pricing information,
# submitting a contest creation request, and getting a purchasing API for the contest creation. Unless otherwise
# stated, all prices are in VOTE.

    getPriceSchedule @0 () -> (schedule :Map(LineItem, Price));
    # Get a list of prices for various aspects of contest creation. Any omitted prices are assumed to be zero.
    getContestLimits @1 () -> (limits :Map(ContestLimit, LimitValue));
    # Get a list of limits for various aspects of contest creation. Any omitted limits are assumed to be unlimited.

    purchaseContest @2 (request :ContestCreationRequest) -> (purchaseApi :Purchase);
    # Begin checkout with a contest creation request. Returns a Purchase API to finish checkout.

    enum LineItems {
    # LineItems is an enumeration of all the individual features we may charge for
    # NOTE: updates to this enum should be reflected in VotingApp/wrappers/ContestCreationRequest.hpp
        contestTypeOneOfN @0;
        # Base price for the One of N contest type

        pluralityTally @1;
        # Price for the plurality method of tallying votes

        contestant3 @2;
        contestant4 @3;
        contestant5 @4;
        contestant6 @5;
        contestant7Plus @6;
        # Prices for more than 2 contestants

        infiniteDurationContest @7;
        # Price for contests which never end
    }
    struct LineItem {
    # Generic types must be pointer types (like a struct), so wrap a LineItems entry in a struct so we can map it.
        item @0 :LineItems;
    }
    struct Price {
    # Similar to LineItem, wrap the price in a struct.
        price @0 :Int64 = 0;
    }

    enum ContestLimits {
    # ContestLimits is an enumeration of all the limits on contest creation.
    # NOTE: updates to this enum should be reflected in VotingApp/wrappers/ContestCreationRequest.hpp
        nameLength @0;
        # Maximum length of the contest name
        descriptionSoftLength @1;
        # Maximum length of the contest description before risking a surcharge
        descriptionHardLength @2;
        # Maximum permissible length of the contest description
        contestantCount @3;
        # Maximum number of contestants
        contestantNameLength @4;
        # Maximum length of a contestant name
        contestantDescriptionSoftLength @5;
        # Maximum length of a contestant description before risking a surcharge
        contestantDescriptionHardLength @6;
        # Maximum permissible length of a contestant description
        maxEndDate @7;
        # Maximum end date of a finite-duration contest. A value of zero allows infinite contest duration; any other
        # value is the millisecond timestamp of the latest allowable end date
    }
    struct ContestLimit {
    # Wrap the ContestLimit in a struct
        limit @0 :ContestLimits;
    }
    struct LimitValue {
    # Wrap the value of the ContestLimit in a struct
        value @0 :Int64;
    }

    struct ContestCreationRequest {
    # This contains all of the information necessary to request a contest be created and get a complete price.

        contestOptions @0 :ContestOptions;
        creatorSignature @1 :ContestCreatorSignature;
        # If the creator wishes to publicly state that he created this contest, he must populate creatorSignature with
        # his memo key's signature on contestOptions. Otherwise, he may set anonymous and his identity will not be
        # revealed.
        sponsorship :union {
            noSponsorship @2 :Void;
            options :group {
                maxVotes @3 :Int64;
                # Maximum number of votes to sponsor; zero indicates unlimited
                maxRevotes @4 :Int32;
                # Maximum number of revotes per voter to sponsor; zero indicates unlimited
                endDate @5 :Int64;
                # Millisecond timestamp of end of vote sponsorship period
                incentive @6 :Int64;
                # Balance to pay each voter as an incentive for voting on this contest
            }
        }
    }
}
