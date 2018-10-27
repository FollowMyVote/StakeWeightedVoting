import qbs
import qbs.File
import qbs.Environment

Module {
    Depends { name: "cpp" }
    property string graphenePath: Environment.getEnv("GRAPHENE_PATH")
    property bool found: File.exists(graphenePath+"/lib/libgraphene_app.a")
    property bool debug: File.exists(graphenePath+"/lib/libfc_debug.a")
    cpp.includePaths: graphenePath+"/include"
    cpp.libraryPaths: [graphenePath+"/lib", graphenePath+"/lib/cryptonomex"]
    cpp.staticLibraries: [
        "boost_system",
        "boost_filesystem",
        "boost_program_options",
        "boost_chrono",
        "boost_date_time",
        "boost_coroutine",
        qbs.hostOS.contains("osx")? "boost_context-mt" : "boost_context",
        qbs.hostOS.contains("osx")? "boost_thread-mt" : "boost_thread",
        "ssl",
        "crypto",
        "graphene_app",
        "graphene_chain",
        "graphene_utilities",
        "graphene_net",
        "graphene_grouped_orders",
        "graphene_egenesis_full",
        "graphene_db",
        "graphene_debug_witness",
        "graphene_account_history",
        "graphene_market_history",
        debug? "fc_debug" : "fc",
        "secp256k1",
        "z"
    ]
}
