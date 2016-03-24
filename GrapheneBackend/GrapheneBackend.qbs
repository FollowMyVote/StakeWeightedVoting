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
