@0xc958eba51b7c4b59;

using ContestCreator = import "/contestcreator.capnp".ContestCreator;
using Map = import "/map.capnp".Map;

struct Config {
    priceSchedule @0 :List(Price);
    # Current price schedule
    contestLimits @1 :List(ContestLimit);
    # Current contest limits
    contestPublishingAccountWif @2 :Text;
    # Private key (WIF format) for the contest publishing account
    authenticatingKeyWif @3 :Text;
    # Private key to authenticate to client with (usually the contest publisher's memo key)

    struct Price {
       lineItem @0 :ContestCreator.LineItems;
       price @1 :Int64;
    }
    struct ContestLimit {
        name @0 :ContestCreator.ContestLimits;
        limit @1 :Int64;
    }
}
