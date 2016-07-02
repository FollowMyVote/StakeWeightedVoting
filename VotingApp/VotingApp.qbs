import qbs
import qbs.FileInfo

QtGuiApplication {
    name: "VotingApp"
    // Only g++ can build against V-Play, some weird issue with the std libs
    enabled: cpp.compilerName === "g++"

    Depends { name: "shared" }
    Depends { name: "StubChainAdaptor" }
    Depends { name: "Qt"; submodules: ["network", "qml", "charts"] }
    Depends { name: "libqtqmltricks-qtquickuielements" }
    Depends { name: "VPlay" }
    // 'original' is a keyword: https://doc.qt.io/qbs/module-item.html#special-property-values
    VPlay.sdkPath: original? original : Qt.core.incPath + "/.."

    qmlImportPaths: [VPlay.sdkPath + "/qml"]
    cpp.cxxLanguageVersion: "c++14"
    cpp.includePaths: [".", "qml-promise/src", VPlay.includePath]
    cpp.libraryPaths: VPlay.sdkPath + "/lib"
    cpp.staticLibraries: VPlay.staticLibrary

    files: [
        "BitsharesWalletBridge.cpp",
        "BitsharesWalletBridge.hpp",
        "PromiseConverter.cpp",
        "PromiseConverter.hpp",
        "TwoPartyClient.cpp",
        "TwoPartyClient.hpp",
        "VotingSystem.cpp",
        "VotingSystem.hpp",
        "Converters.hpp",
        "capnqt/QSocketWrapper.cpp",
        "capnqt/QSocketWrapper.hpp",
        "capnqt/QtEventPort.cpp",
        "capnqt/QtEventPort.hpp",
        "main.cpp",
        "qml.qrc",
        "qml/*.qml",
        "qml/CustomControls/*",
        "DataStructures/*.cpp",
        "DataStructures/*.hpp",
        "DataStructures/README.md",
        "Apis/*.cpp",
        "Apis/*.hpp",
        "Apis/README.md",
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
