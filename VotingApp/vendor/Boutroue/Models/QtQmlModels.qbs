import qbs;

Project {
    name: "Qt QML Models";

    Product {
        name: "libqtqmltricks-qtqmlmodels";
        type: "staticlibrary";
        targetName: "QtQmlModels";
        cpp.cxxLanguageVersion: "c++14"
        cpp.cxxStandardLibrary: qbs.hostOS.contains("osx")? "libc++" : "libstdc++"; // NOTE : because there are issues with libc++

        Export {
            cpp.includePaths: ".";
            cpp.cxxLanguageVersion: "c++14"
            cpp.cxxStandardLibrary: qbs.hostOS.contains("osx")? "libc++" : "libstdc++"; // NOTE : because there are issues with libc++

            Depends { name: "cpp"; }
            Depends {
                name: "Qt";
                submodules: ["core", "qml"];
            }
        }
        Depends { name: "cpp"; }
        Depends {
            name: "Qt";
                submodules: ["core", "qml"];
        }
        Group {
            name: "C++ sources";
            files: [
                "QQmlObjectListModel.cpp",
                "QQmlVariantListModel.cpp",
            ]
        }
        Group {
            name: "C++ headers";
            files: [
                "QQmlObjectListModel.h",
                "QQmlVariantListModel.h",
                "QtQmlTricksPlugin_SmartDataModels.h",
            ]
        }
        Group {
            qbs.install: (product.type === "dynamiclibrary");
            fileTagsFilter: product.type;
        }
    }
}
