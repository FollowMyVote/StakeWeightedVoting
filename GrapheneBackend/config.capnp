@0xc958eba51b7c4b59;

using ContestCreator = import "/contestcreator.capnp".ContestCreator;
using Map = import "/map.capnp".Map;

struct Config {
    priceSchedule @0 :List(Price);
    contestLimits @1 :List(ContestLimit);

    struct Price {
       lineItem @0 :ContestCreator.LineItems;
       price @1 :Int64;
    }
    struct ContestLimit {
        name @0 :ContestCreator.ContestLimits;
        limit @1 :Int64;
    }
}
