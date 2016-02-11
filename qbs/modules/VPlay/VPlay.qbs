import qbs

Module {
  name: "VPlay"

  // include vplay header, lib and resources
  property string sdkPath: qbs.getEnv("VPLAY_SDK_PATH")
  property string qtIncPath: sdkPath + "/include"
  property string includePath: qtIncPath+"/VPlay"
  property string staticLibrary: "VPlay"
                       + (qbs.targetOS.contains("ios-simulator") ? "_iphonesimulator" : "")
                       + (qbs.buildVariant === "debug" ? "_debug" : "")

  // add required Qt dependencies
  Depends {
    name: "Qt"
    submodules: ["qml", "quick", "widgets", "network", "multimedia", "sql", "websockets"]
  }

  // add 3D dependencies on supported platforms
  Depends {
    condition: !(qbs.targetOS.contains("winrt") || qbs.targetOS.contains("windowsphone") || qbs.targetOS.contains("ios"))
    name: "Qt"
    submodules: ["3dcore", "3drender", "3dinput", "3dquick"]
  }

  Group {
    name: "vplay-license"
    files: VPlay.sdkPath+"/mkspecs/common/vplay/resources_user.qrc"
  }
}
