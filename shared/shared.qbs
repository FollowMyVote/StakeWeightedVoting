import qbs

StaticLibrary {
    name: "shared"

    Depends { name: "cpp" }
    Depends { name: "capnp" }
    Depends { name : "botan" }

    cpp.cxxLanguageVersion: "c++14"
    cpp.cxxStandardLibrary: qbs.hostOS.contains("osx") ? "libc++" : "libstdc++"
    cpp.dynamicLibraries: [].concat(capnp.dynamicLibraries).concat(botan.dynamicLibraries)
    cpp.includePaths: ["capnp"].concat(botan.includePaths).concat(capnp.includePaths)
    cpp.libraryPaths: [].concat(botan.libraryPaths).concat(capnp.libraryPaths)

    files: [
        "Utilities.hpp",
        "BotanIntegration/TlsPskAdaptor.cpp",
        "BotanIntegration/TlsPskAdaptor.hpp",
        "BotanIntegration/TlsPskAdaptorFactory.cpp",
        "BotanIntegration/TlsPskAdaptorFactory.hpp",
        "capnp/*.capnp",
    ]

    Export {
        Depends { name : "cpp" }
        Depends { name : "capnp" }
        Depends { name : "botan" }
        cpp.cxxLanguageVersion: "c++14"
        cpp.cxxStandardLibrary: qbs.hostOS.contains("osx") ? "libc++" : "libstdc++"
        cpp.cxxFlags: capnp.cxxFlags
        cpp.dynamicLibraries: [].concat(capnp.dynamicLibraries).concat(botan.dynamicLibraries)
        cpp.includePaths: [".", "capnp"].concat(botan.includePaths).concat(capnp.includePaths)
        cpp.libraryPaths: [].concat(botan.libraryPaths).concat(capnp.libraryPaths)
    }
}
