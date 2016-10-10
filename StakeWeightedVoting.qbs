import qbs

Project {
    id: project
    minimumQbsVersion: 1.6

    property bool ignoreApp: false

    qbsSearchPaths: "qbs"
    references: {
        var baseList = ["shared", "GrapheneBackend"]
        if (!ignoreApp)
            baseList = baseList.concat(["VotingApp"])
        return baseList
    }
}
