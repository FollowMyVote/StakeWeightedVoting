import qbs
import qbs.Probes

Module {
    // Outputs
    property bool foundBotan: {
        if (!botanProbe.found)
            throw "Unable to find botan. Try setting PKG_CONFIG_PATH to ensure botan can be found."
        return true
    }
    property var cxxFlags: botanProbe.cflags
    property var dynamicLibraries: botanProbe.libs.filter(function(name) {
        return name.startsWith("-l")
    }).map(function(name) { return name.slice(2) })

    Probes.PkgConfigProbe {
        id: botanProbe
        name: "botan-1.11"
    }
}
