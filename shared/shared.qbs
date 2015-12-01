import qbs
import qbs.Probes

StaticLibrary {
    name: "shared"

    Depends { name: "cpp" }
    cpp.includePaths: ["capnp"]
    cpp.cxxLanguageVersion: "c++14"
    cpp.cxxStandardLibrary: "libc++"
    Depends { name: "Qt"; submodules: ["qml"] }
    Depends { name: "capnp" }

    Probes.PkgConfigProbe {
        id: capnpProbe
        name: "capnp-rpc"
    }
    property bool foundCapnp: {
        if (!capnpProbe.found)
            throw "Unable to find capnp. Try setting PATH and PKG_CONFIG_PATH to ensure capnp can be found."
        return true
    }

    cpp.cxxFlags: capnpProbe.cflags
    cpp.linkerFlags: capnpProbe.libs

    files: [
        "BlockchainAdaptorInterface.hpp",
        "capnp/*.capnp",
    ]

    Export {
        Depends { name : "cpp" }
        cpp.includePaths: [".", "capnp"]
        cpp.cxxLanguageVersion: "c++14"
        cpp.cxxStandardLibrary: "libc++"
        cpp.cxxFlags: capnpProbe.cflags
        cpp.linkerFlags: capnpProbe.libs
    }
}
