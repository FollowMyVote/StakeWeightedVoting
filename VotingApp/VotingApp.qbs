import qbs
import qbs.FileInfo

QtGuiApplication {
    name: "VotingApp"

    Depends { name: "shared" }
    Depends { name: "StubChainAdaptor" }
    Depends { name: "Qt"; submodules: ["network", "qml"] }
    Depends { name: "VPlay" }
    VPlay.sdkPath: original? original : Qt.incPath + "/.."

    qmlImportPaths: [VPlay.qtIncPath + "/qml"]
    cpp.includePaths: [".", "qml-promise/src", VPlay.includePath]
    cpp.libraryPaths: VPlay.sdkPath + "/lib"
    cpp.staticLibraries: VPlay.staticLibrary

    files: [
        "PromiseConverter.cpp",
        "PromiseConverter.hpp",
        "TwoPartyClient.cpp",
        "TwoPartyClient.hpp",
        "VotingSystem.cpp",
        "VotingSystem.hpp",
        "capnqt/QSocketWrapper.cpp",
        "capnqt/QSocketWrapper.hpp",
        "capnqt/QtEventPort.cpp",
        "capnqt/QtEventPort.hpp",
        "main.cpp",
        "qml.qrc",
        "qml/*.qml",
        "wrappers/ChainAdaptorWrapper.cpp",
        "wrappers/ChainAdaptorWrapper.hpp",
        "wrappers/BackendWrapper.cpp",
        "wrappers/BackendWrapper.hpp",
        "wrappers/Balance.cpp",
        "wrappers/Balance.hpp",
        "wrappers/Coin.cpp",
        "wrappers/Coin.hpp",
        "wrappers/Contest.cpp",
        "wrappers/Contest.hpp",
        "wrappers/ContestCreator.cpp",
        "wrappers/ContestCreator.hpp",
        "wrappers/ContestGeneratorWrapper.cpp",
        "wrappers/ContestGeneratorWrapper.hpp",
        "wrappers/Datagram.cpp",
        "wrappers/Datagram.hpp",
        "wrappers/Decision.cpp",
        "wrappers/Decision.hpp",
        "wrappers/OwningWrapper.cpp",
        "wrappers/OwningWrapper.hpp",
        "wrappers/Converters.hpp",
        "wrappers/PurchaseContestRequest.cpp",
        "wrappers/PurchaseContestRequest.hpp",
        "wrappers/PurchaseWrapper.cpp",
        "wrappers/PurchaseWrapper.hpp",
        "wrappers/README.md",
        "qml-promise/src/Promise.cpp",
        "qml-promise/src/Promise.hpp",
        "vendor/QQmlVariantListModel.cpp",
        "vendor/QQmlVariantListModel.h",
        "vendor/QQmlObjectListModel.cpp",
        "vendor/QQmlObjectListModel.h",
        "vendor/QQmlEnumClassHelper.h",
    ]

    property bool install: true
    property string installDir: bundle.isBundle ? "Applications" : (qbs.targetOS.contains("windows") ? "" : "bin")

    Group {
        fileTagsFilter: ["application"]
        qbs.install: install
        qbs.installDir: bundle.isBundle ? FileInfo.joinPaths(installDir, FileInfo.path(bundle.executablePath))
                                        : installDir
    }

    Group {
        fileTagsFilter: ["infoplist"]
        qbs.install: install && bundle.isBundle && !bundle.embedInfoPlist
        qbs.installDir: FileInfo.joinPaths(installDir, FileInfo.path(bundle.infoPlistPath))
    }

    Group {
        fileTagsFilter: ["pkginfo"]
        qbs.install: install && bundle.isBundle
        qbs.installDir: FileInfo.joinPaths(installDir, FileInfo.path(bundle.pkgInfoPath))
    }
}
