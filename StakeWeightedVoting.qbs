import qbs

Project {
    qbsSearchPaths: "qbs"
    references: ["shared", "StubBackend", "StubChainAdaptor", "VotingApp", "GrapheneBackend", "vendor/qt-quick-ui-elements"]
}
