import qbs

CppApplication {
    name: "GrapheneBackend"
    consoleApplication: true
    condition: graphene.found

    Depends { name: "shared" }
    Depends { name: "graphene" }

    files: [
        "main.cpp",
    ]

    Group {
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "bin"
    }
}
