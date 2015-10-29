import qbs

StaticLibrary {
    name: "shared"

    Depends { name: "cpp" }
    cpp.includePaths: ["capnp"]
    cpp.cxxLanguageVersion: "c++14"
    cpp.cxxStandardLibrary: "libc++"
    cpp.dynamicLibraries: ["kj", "kj-async", "capnp", "capnp-rpc"]
    Depends { name: "capnp" }
    Depends { name: "Qt"; submodules: ["qml"] }

    files: [
        "BlockchainAdaptorInterface.hpp",
        "capnp/*.capnp",
    ]

    Export {
        Depends { name : "cpp" }
        cpp.includePaths: [".", "capnp"]
        cpp.cxxLanguageVersion: "c++14"
        cpp.cxxStandardLibrary: "libc++"
        cpp.dynamicLibraries: ["kj", "kj-async", "capnp", "capnp-rpc"]
    }
}
