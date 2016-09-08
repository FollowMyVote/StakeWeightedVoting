import qbs
import qbs.FileInfo
import qbs.Probes

Project {
    QtGuiApplication {
        name: "VotingApp"

        Depends { name: "shared" }
        Depends { name: "Qt"; submodules: ["network", "qml", "charts", "websockets", "quickcontrols2"] }
        Depends { name: "libqtqmltricks-qtquickuielements" }
        Depends { name: "libqtqmltricks-qtsupermacros" }
        Depends { name: "libqtqmltricks-qtqmlmodels" }

        qmlImportPaths: ["vendor/QuickPromise"]

        cpp.cxxLanguageVersion: "c++14"
        cpp.cxxStandardLibrary: qbs.hostOS.contains("osx")? "libc++" : "libstdc++"
        cpp.includePaths: [".", "vendor/QuickPromise"]

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
            "icons.yml",
            "main.cpp",
            "qml/ConnectionProgressPopup.qml",
            "qml/ContestDelegate.qml",
            "qml/ContestDetailPage.qml",
            "qml/ContestStatusRow.qml",
            "qml/CreateContestPage.qml",
            "qml/FeedPage.qml",
            "qml/NavigationDrawer.qml",
            "qml/ShadowedPopup.qml",
            "qml/WalletConfirmationPopup.qml",
            "qml/main.qml",
            "DataStructures/*.cpp",
            "DataStructures/*.hpp",
            "DataStructures/README.md",
            "Apis/*.cpp",
            "Apis/*.hpp",
            "Apis/README.md",
            "vendor/QuickPromise/qptimer.h",
            "vendor/QuickPromise/qptimer.cpp",
            "vendor/QuickPromise/qppromise.h",
            "vendor/QuickPromise/qppromise.cpp",
            "vendor/QuickPromise/quickpromise.qrc",
        ]

        FileTagger {
            patterns: "icons.yml"
            fileTags: "icon-manifest"
        }
        Rule {
            id: iconFetcher
            inputs: ["icon-manifest"]

            Artifact {
                fileTags: ['qrc']
                filePath: product.sourceDirectory + "/icons/icons.qrc"
            }

            prepare: {
                var cmd = new Command("/usr/bin/env", ["python2", "icons.py", input.filePath])
                cmd.workingDirectory = product.sourceDirectory
                cmd.description = "Downloading icons for " + input.fileName
                cmd.highlight = "filegen"
                return cmd
            }
        }

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

    SubProject {
        filePath: "vendor/Boutroue/UiElements/QtQuickUiElements.qbs"
    }
    SubProject {
        filePath: "vendor/Boutroue/SuperMacros/QtSuperMacros.qbs"
    }
    SubProject {
        filePath: "vendor/Boutroue/Models/QtQmlModels.qbs"
    }
}
