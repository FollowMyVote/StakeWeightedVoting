import qbs

CppApplication {
    name: "GrapheneBackend"
    consoleApplication: true
    condition: graphene.found

    Depends { name: "shared" }
    Depends { name: "graphene" }

    files: [
        "BackendPlugin.cpp",
        "BackendPlugin.hpp",
        "BackendServer.cpp",
        "BackendServer.hpp",
        "CoinVolumeHistory.cpp",
        "CoinVolumeHistory.hpp",
        "Contest.cpp",
        "Contest.hpp",
        "ContestResultsServer.cpp",
        "ContestResultsServer.hpp",
        "CustomEvaluator.cpp",
        "CustomEvaluator.hpp",
        "Decision.cpp",
        "Decision.hpp",
        "FeedGenerator.cpp",
        "FeedGenerator.hpp",
        "Types.hpp",
        "Utilities.hpp",
        "VoteDatabase.cpp",
        "VoteDatabase.hpp",
        "compat/FcEventPort.cpp",
        "compat/FcEventPort.hpp",
        "compat/FcStreamWrapper.cpp",
        "compat/FcStreamWrapper.hpp",
        "main.cpp",
    ]

    Group {
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "bin"
    }
}
