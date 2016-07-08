import qbs

StaticLibrary {
    name: "shared"

    Depends { name: "cpp" }
    Depends { name: "capnp" }
    Depends { name: "botan" }

    cpp.includePaths: ["capnp"]
    cpp.cxxLanguageVersion: "c++14"
    cpp.cxxStandardLibrary: qbs.hostOS.contains("osx") ? "libc++" : "libstdc++"
    cpp.cxxFlags: [].concat(capnp.cxxFlags, botan.cxxFlags).filter(function(x) { return !!x })
    cpp.dynamicLibraries: [].concat(capnp.dynamicLibraries, botan.dynamicLibraries)

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
        cpp.cxxFlags: [].concat(capnp.cxxFlags, botan.cxxFlags).filter(function(x) { return !!x })
        cpp.dynamicLibraries: [].concat(capnp.dynamicLibraries, botan.dynamicLibraries)
    }
}
