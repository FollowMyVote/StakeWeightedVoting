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
        "main.cpp",
    ]

    Group {
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "bin"
    }
}
