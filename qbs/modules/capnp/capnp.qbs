import qbs
import qbs.File
import qbs.Probes
import qbs.FileInfo
import qbs.ModUtils
import qbs.Environment

Module {
    // Inputs
    property string capnpPath: qbs.hostOS.contains("osx")? "/usr/local/bin" : "/usr/bin"
    property var importPaths: []

    // Outputs
    property bool foundCapnp: {
        if (!capnpProbe.found)
            throw "Unable to find capnp. Try setting PATH and PKG_CONFIG_PATH to ensure capnp can be found."
        return true
    }
    property var cxxFlags: capnpProbe.cflags
    property var dynamicLibraries: capnpProbe.libs.filter(function(name) { return name.startsWith("-l") }).map(function(name) { return name.slice(2) })

    PropertyOptions {
        name: "capnpPath"
        description: "Path to the capnp executables"
    }
    PropertyOptions {
        name: "importPaths"
        description: "Directories to search for non-relative imports"
    }

    Probes.PkgConfigProbe {
        id: capnpProbe
        name: "capnp-rpc"
    }

    validate: {
        // Check if an absolute path to capnp was provided
        if (FileInfo.isAbsolutePath(capnpPath) && File.exists(capnpPath) + "/capnp" &&
                File.exists(FileInfo.path(capnpPath) + "/capnpc-c++")) {
            return
        }

        var delimiter = qbs.hostOS.contains("windows")? ";" : ":"
        var paths = Environment.getEnv("PATH").split(delimiter)
        if (qbs.hostOS === "osx")
            paths.push("/usr/local/bin")

        // Check if capnp is in PATH
        for (var i = 0; i < paths.length; ++i)
            if (File.exists(paths[i] + "/capnp") && File.exists(paths[i] + "/capnpc-c++"))
                return

        throw "Unable to find capnp binaries. Please set capnpPath property."
    }

    FileTagger {
        patterns: ["*.capnp"]
        fileTags: ["capnp"]
    }
    Rule {
        inputs: ["capnp"]
        Artifact {
            filePath: input.filePath + ".c++"
            fileTags: ["cpp"]
        }
        Artifact {
            filePath: input.filePath + ".h"
            fileTags: ["hpp"]
        }

        prepare: {
            var command = "capnp"
            var compilerOption = "-oc++"
            var importPaths = product.moduleProperty("capnp", "importPaths").map(function(path) { return "-I" + path })
            if (product.moduleProperty("capnp", "capnpPath") !== "") {
                command = product.moduleProperty("capnp", "capnpPath") + "/capnp"
                compilerOption = "-o" + product.moduleProperty("capnp", "capnpPath") + "/capnpc-c++"
            }

            var args = ["compile", compilerOption].concat(importPaths).concat([input.filePath]);
            var cmd = new Command(command, args);
            cmd.description = "Generating Cap'n Proto code for " + input.fileName;
            cmd.highlight = "codegen";
            cmd.workingDirectory = FileInfo.path(input.filePath)
            return cmd;
        }
    }
}
