import qbs

StaticLibrary {
    name: "shared"

    Depends { name: "cpp" }
    cpp.includePaths: ["capnp"]
    cpp.cxxLanguageVersion: "c++14"
    cpp.cxxStandardLibrary: qbs.hostOS.contains("osx") ? "libc++" : "libstdc++"
    Depends { name: "Qt"; submodules: ["qml"] }
    Depends { name: "capnp" }

    cpp.cxxFlags: capnp.cflags
    cpp.dynamicLibraries: capnp.dynamicLibraries

    files: [
        "TwoPartyServer.cpp",
        "TwoPartyServer.hpp",
        "Utilities.hpp",
        "capnp/*.capnp",
    ]

    Export {
        Depends { name : "cpp" }
        Depends { name : "capnp" }
        cpp.includePaths: [".", "capnp"]
        cpp.cxxLanguageVersion: "c++14"
        cpp.cxxStandardLibrary: qbs.hostOS.contains("osx") ? "libc++" : "libstdc++"
        cpp.cxxFlags: capnp.cflags
        cpp.dynamicLibraries: capnp.dynamicLibraries
    }
}
