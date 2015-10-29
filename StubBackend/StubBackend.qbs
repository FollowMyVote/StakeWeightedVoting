import qbs

QtApplication {
    name: "StubBackend"
    consoleApplication: true

    Depends { name: "shared" }

    files: [
        "BackendServer.cpp",
        "BackendServer.hpp",
        "TwoPartyServer.cpp",
        "TwoPartyServer.hpp",
        "main.cpp",
    ]

    Group {
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "bin"
    }
}
