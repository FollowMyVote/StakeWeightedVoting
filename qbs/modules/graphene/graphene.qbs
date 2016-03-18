import qbs
import qbs.File

Module {
    Depends { name: "cpp" }
    property string graphenePath: qbs.getEnv("GRAPHENE_PATH")
    property bool found: File.exists(graphenePath+"/lib/libgraphene_app.a")
    cpp.includePaths: graphenePath+"/include"
    cpp.libraryPaths: graphenePath+"/lib"
    cpp.staticLibraries: ["fc", "graphene_app","graphene_chain"]
}
