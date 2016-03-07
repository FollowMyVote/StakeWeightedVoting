import qbs
import qbs.FileInfo

DynamicLibrary {
    name: "StubChainAdaptor"

    Depends { name: "cpp" }
    Depends { name: "shared" }
    Depends { name: "Qt"; submodules: ["core"] }

    files: [
        "BackendStub.cpp",
        "BackendStub.hpp",
        "ContestCreator.cpp",
        "ContestCreator.hpp",
        "ContestGenerator.cpp",
        "ContestGenerator.hpp",
        "ContestResults.cpp",
        "ContestResults.hpp",
        "StubChainAdaptor.cpp",
        "StubChainAdaptor.hpp",
        "Purchase.cpp",
        "Purchase.hpp",
        "StubChainAdaptor_global.hpp",
    ]

    property bool install: true
    property string installDir: bundle.isBundle ? "Library/Frameworks" : (qbs.targetOS.contains("windows") ? "" : "lib")

    Group {
        fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_symlink", "dynamiclibrary_import"]
        qbs.install: install
        qbs.installDir: bundle.isBundle ? FileInfo.joinPaths(installDir, FileInfo.path(bundle.executablePath)) : installDir
    }

    Group {
        fileTagsFilter: ["infoplist"]
        qbs.install: install && bundle.isBundle && !bundle.embedInfoPlist
        qbs.installDir: FileInfo.joinPaths(installDir, FileInfo.path(bundle.infoPlistPath))
    }

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: ["."]
    }
}
