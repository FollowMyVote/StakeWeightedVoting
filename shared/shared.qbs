import qbs

StaticLibrary {
    name: "shared"

    Depends { name: "cpp" }
    Depends { name: "capnp" }

    cpp.includePaths: ["capnp"]
    cpp.cxxLanguageVersion: "c++14"
    cpp.cxxStandardLibrary: qbs.hostOS.contains("osx") ? "libc++" : "libstdc++"
    cpp.cxxFlags: capnp.cxxFlags
    cpp.dynamicLibraries: capnp.dynamicLibraries

    files: [
        "Utilities.hpp",
        "capnp/*.capnp",
    ]

    Export {
        Depends { name : "cpp" }
        Depends { name : "capnp" }
        Depends { name : "botan" }
        cpp.includePaths: [".", "capnp"]
        cpp.cxxLanguageVersion: "c++14"
        cpp.cxxStandardLibrary: qbs.hostOS.contains("osx") ? "libc++" : "libstdc++"
        cpp.cxxFlags: capnp.cxxFlags
        cpp.dynamicLibraries: capnp.dynamicLibraries
    }
}
