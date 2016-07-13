import qbs

CppApplication {
    name: "GrapheneBackend"
    consoleApplication: true
    // Only clang can build the GrapheneBackend; g++ can't handle boost
    condition: graphene.found && cpp.compilerName === "clang++"
    cpp.cxxFlags: "-fno-limit-debug-info"
    cpp.includePaths: "."

    Depends { name: "shared" }
    Depends { name: "graphene" }
    Depends { name: "capnp" }
    capnp.importPaths: ["../shared/capnp"]

    files: [
        "BackendConfiguration.cpp",
        "BackendConfiguration.hpp",
        "VoteDatabase.cpp",
        "VoteDatabase.hpp",
        "GrapheneIntegration/BackendPlugin.cpp",
        "GrapheneIntegration/BackendPlugin.hpp",
        "GrapheneIntegration/CustomEvaluator.cpp",
        "GrapheneIntegration/CustomEvaluator.hpp",
        "ApiServers/BackendServer.cpp",
        "ApiServers/BackendServer.hpp",
        "ApiServers/ContestCreatorServer.cpp",
        "ApiServers/ContestCreatorServer.hpp",
        "ApiServers/ContestResultsServer.cpp",
        "ApiServers/ContestResultsServer.hpp",
        "ApiServers/FeedGenerator.hpp",
        "Objects/Objects.hpp",
        "Objects/CoinVolumeHistory.cpp",
        "Objects/CoinVolumeHistory.hpp",
        "Objects/Contest.cpp",
        "Objects/Contest.hpp",
        "Objects/Decision.cpp",
        "Objects/Decision.hpp",
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
