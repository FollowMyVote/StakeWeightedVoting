import qbs;

Project {
    name: "Qt super-macros";

    Product {
        name: "libqtqmltricks-qtsupermacros";
        type: "staticlibrary";
        targetName: "QtSuperMacros";
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
                "QQmlHelpers.cpp",
            ]
        }
        Group {
            name: "C++ headers";
            files: [
                "QQmlConstRefPropertyHelpers.h",
                "QQmlEnumClassHelper.h",
                "QQmlListPropertyHelper.h",
                "QQmlPtrPropertyHelpers.h",
                "QQmlVarPropertyHelpers.h",
            ]
        }
        Group {
            qbs.install: (product.type === "dynamiclibrary");
            fileTagsFilter: product.type;
        }
    }
}
