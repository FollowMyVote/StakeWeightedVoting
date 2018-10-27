import qbs
import qbs.Probes

Module {
    // Outputs
    property bool foundBotan: {
        if (!botanProbe.found)
            throw "Unable to find botan. Try setting PKG_CONFIG_PATH to ensure botan can be found."
        return true
    }
    property var includePaths: botanProbe.cflags.filter(function(f){
        return f.startsWith("-I")
    }).map(function(f){return f.slice(2)})
    property var dynamicLibraries: botanProbe.libs.filter(function(name) {
        return name.startsWith("-l")
    }).map(function(name) { return name.slice(2) })
    property var libraryPaths: botanProbe.libs.filter(function(name) {
        return name.startsWith("-L")
    }).map(function(name) { return name.slice(2) })


    Probes.PkgConfigProbe {
        id: botanProbe
        name: "botan-2"
    }
}
