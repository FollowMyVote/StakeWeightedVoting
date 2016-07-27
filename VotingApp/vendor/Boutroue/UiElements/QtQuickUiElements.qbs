import qbs;

Project {
    name: "QtQuick UI Elements";

    property bool noQtPrivateLibs : false;

    Product {
        name: "libqtqmltricks-qtquickuielements";
        type: "staticlibrary";
        targetName: "QtQuickUiElements";
        cpp.defines: {
            var ret = [];
            if (project.noQtPrivateLibs) {
                ret.push ("NO_QT_PRIVATE_LIBS=1");
            }
            return ret;
        }
        cpp.cxxLanguageVersion: (Qt.core.versionMajor >= 5 && Qt.core.versionMinor >= 7 ? "c++11" : "c++98");
        cpp.cxxStandardLibrary: "libstdc++"; // NOTE : because there are issues with libc++

        readonly property stringList qmlImportPaths : [sourceDirectory + "/imports"]; // equivalent to QML_IMPORT_PATH += $$PWD/imports
        readonly property stringList qtModules : {
             var ret = ["core", "gui", "qml", "quick", "svg"];
             if (!project.noQtPrivateLibs) {
                 ret.push ("core-private");
                 ret.push ("qml-private");
             }
             return ret;
         }

        Export {
            cpp.includePaths: ".";
            cpp.cxxLanguageVersion: (Qt.core.versionMajor >= 5 && Qt.core.versionMinor >= 7 ? "c++11" : "c++98");
            cpp.cxxStandardLibrary: "libstdc++"; // NOTE : because there are issues with libc++

            Depends { name: "cpp"; }
            Depends {
                name: "Qt";
                submodules: product.qtModules;
            }
        }
        Depends { name: "cpp"; }
        Depends {
            name: "Qt";
            submodules: product.qtModules;
        }
        Group {
            name: "C++ sources";
            files: [
                "QQmlFsSingleton.cpp",
                "QQmlIntrospector.cpp",
                "QQmlMimeIconsHelper.cpp",
                "QQuickEllipse.cpp",
                "QQuickExtraAnchors.cpp",
                "QQuickFormContainer.cpp",
                "QQuickGridContainer.cpp",
                "QQuickPixelPerfectContainer.cpp",
                "QQuickPolygon.cpp",
                "QQuickRoundedRectanglePaintedItem.cpp",
                "QQuickStretchColumnContainer.cpp",
                "QQuickStretchRowContainer.cpp",
                "QQuickSvgIconHelper.cpp",
                "QQuickThemeIconProvider.cpp",
                "QQuickWrapLeftRightContainer.cpp",
            ]
        }
        Group {
            name: "C++ headers";
            files: [
                "QQmlFsSingleton.h",
                "QQmlIntrospector.h",
                "QQmlMimeIconsHelper.h",
                "QQuickEllipse.h",
                "QQuickExtraAnchors.h",
                "QQuickFormContainer.h",
                "QQuickGridContainer.h",
                "QQuickPixelPerfectContainer.h",
                "QQuickPolygon.h",
                "QQuickRoundedRectanglePaintedItem.h",
                "QQuickStretchColumnContainer.h",
                "QQuickStretchRowContainer.h",
                "QQuickSvgIconHelper.h",
                "QQuickThemeIconProvider.h",
                "QQuickWrapLeftRightContainer.h",
                "QtQmlTricksPlugin.h",
                "QtQmlTricksPlugin_UiElements.h",
            ]
        }
        Group {
            name: "Qt resources bundle";
            files: [
                "qtqmltricks_uielements.qrc",
                "qtqmltricks_svgicons_actions.qrc",
                "qtqmltricks_svgicons_devices.qrc",
                "qtqmltricks_svgicons_filetypes.qrc",
                "qtqmltricks_svgicons_others.qrc",
                "qtqmltricks_svgicons_services.qrc",
            ]
        }
        Group {
            name: "Markdown documents";
            files: [
                "README.md",
            ]
        }
        Group {
            qbs.install: (product.type === "dynamiclibrary");
            fileTagsFilter: product.type;
        }
    }
}
