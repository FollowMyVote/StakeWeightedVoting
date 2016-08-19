import qbs

Project {
    id: project

    property bool ignoreApp: false

    qbsSearchPaths: "qbs"
    references: {
        var baseList = ["shared", "GrapheneBackend"]
        if (!ignoreApp)
            baseList = baseList.concat(["StubChainAdaptor", "VotingApp"])
        return baseList
    }
}
