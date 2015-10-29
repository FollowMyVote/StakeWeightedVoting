import qbs

Project {
    qbsSearchPaths: "qbs"
    references: ["shared", "StubBackend", "StubChainAdaptor", "VotingApp"]
}
