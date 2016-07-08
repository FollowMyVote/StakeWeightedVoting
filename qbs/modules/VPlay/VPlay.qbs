import qbs
import qbs.Environment

Module {
  name: "VPlay"

  Depends { name: "cpp" }

  // add required Qt dependencies
  Depends {
    name: "Qt"
    submodules: ["core", "qml", "quick", "widgets", "network", "multimedia", "sql", "websockets"]
  }

  // add 3D dependencies on supported platforms
  Depends {
    condition: !(qbs.targetOS.contains("winrt") || qbs.targetOS.contains("windowsphone") || qbs.targetOS.contains("ios"))
    name: "Qt"
    submodules: ["3dcore", "3drender", "3dinput", "3dquick"]
  }

  // include vplay header, lib and resources
  property string sdkPath: Environment.getEnv("VPLAY_PATH")? Environment.getEnv("VPLAY_PATH")
                                                             // Attempt a guess at V-Play's location
                                                           : "~/V-PlaySDK/"+ Qt.core.versionMajor + "." +
                                                             Qt.core.versionMinor + "/" +
                                                             (cpp.compilerName.startsWith("clang")? "clang" : "gcc") +
                                                             "_64"
  property string qtIncPath: sdkPath + "/include"
  property string includePath: qtIncPath+"/VPlay"
  property string staticLibrary: "VPlay" + (qbs.targetOS.contains("ios-simulator") ? "_iphonesimulator" : "")

  Group {
    name: "vplay-license"
    files: VPlay.sdkPath+"/mkspecs/common/vplay/resources_user.qrc"
  }
}
