import qbs

Project {
    qbsSearchPaths: "qbs"
    references: ["shared", "StubBackend", "StubChainAdaptor", "VotingApp", "vendor/qt-quick-ui-elements"]
}
