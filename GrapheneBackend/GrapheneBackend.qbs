import qbs

CppApplication {
    name: "GrapheneBackend"
    consoleApplication: true
    // Only clang can build the GrapheneBackend; g++ can't handle boost
    condition: graphene.found && cpp.compilerName === "clang++"
    cpp.cxxFlags: "-fno-limit-debug-info"

    Depends { name: "shared" }
    Depends { name: "graphene" }
    Depends { name: "capnp" }
    capnp.importPaths: ["../shared/capnp"]

    files: [
        "BackendConfiguration.cpp",
        "BackendConfiguration.hpp",
        "BackendPlugin.cpp",
        "BackendPlugin.hpp",
        "BackendServer.cpp",
        "BackendServer.hpp",
        "CoinVolumeHistory.cpp",
        "CoinVolumeHistory.hpp",
        "Contest.cpp",
        "Contest.hpp",
        "ContestCreatorServer.cpp",
        "ContestCreatorServer.hpp",
        "ContestResultsServer.cpp",
        "ContestResultsServer.hpp",
        "CustomEvaluator.cpp",
        "CustomEvaluator.hpp",
        "Decision.cpp",
        "Decision.hpp",
        "FeedGenerator.hpp",
        "Types.hpp",
        "VoteDatabase.cpp",
        "VoteDatabase.hpp",
        "compat/FcEventPort.cpp",
        "compat/FcEventPort.hpp",
        "compat/FcStreamWrapper.cpp",
        "compat/FcStreamWrapper.hpp",
        "main.cpp",
        "config.capnp",
    ]

    Group {
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "bin"
    }
}
