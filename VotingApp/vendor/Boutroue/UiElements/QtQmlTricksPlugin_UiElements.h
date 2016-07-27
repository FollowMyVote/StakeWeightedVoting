#ifndef QTQMLTRICKSPLUGIN_UIELEMENTS_H
#define QTQMLTRICKSPLUGIN_UIELEMENTS_H

#include <QQmlEngine>
#include <QDebug>
#include <qqml.h>

#include "QQuickPolygon.h"
#include "QQuickEllipse.h"
#include "QQuickExtraAnchors.h"
#include "QQuickSvgIconHelper.h"
#include "QQmlMimeIconsHelper.h"
#include "QQuickThemeIconProvider.h"
#include "QQuickGridContainer.h"
#include "QQuickFormContainer.h"
#include "QQuickStretchRowContainer.h"
#include "QQuickStretchColumnContainer.h"
#include "QQuickWrapLeftRightContainer.h"
#include "QQuickRoundedRectanglePaintedItem.h"
#include "QQuickPixelPerfectContainer.h"
#include "QQmlIntrospector.h"
#include "QQmlFsSingleton.h"

static void registerQtQmlTricksUiElements (QQmlEngine * engine = Q_NULLPTR) {
    Q_INIT_RESOURCE (qtqmltricks_uielements);
    Q_INIT_RESOURCE (qtqmltricks_svgicons_actions);
    Q_INIT_RESOURCE (qtqmltricks_svgicons_devices);
    Q_INIT_RESOURCE (qtqmltricks_svgicons_filetypes);
    Q_INIT_RESOURCE (qtqmltricks_svgicons_others);
    Q_INIT_RESOURCE (qtqmltricks_svgicons_services);

    const char * uri = "QtQmlTricks.UiElements"; // @uri QtQmlTricks.UiElements
    const int    maj = 2;
    const int    min = 0;

    // shapes
    qmlRegisterType<QQuickEllipse>                       (uri, maj, min, "Ellipse");
    qmlRegisterType<QQuickPolygon>                       (uri, maj, min, "Polygon");
    qmlRegisterType<QQuickRoundedRectanglePaintedItem>   (uri, maj, min, "RoundedRectangle");

    // icons
    qmlRegisterType<QQuickSvgIconHelper>                 (uri, maj, min, "SvgIconHelper");
    qmlRegisterType<QQmlMimeIconsHelper>                 (uri, maj, min, "MimeIconsHelper");

    // layouts
    qmlRegisterType<QQuickGridContainer>                 (uri, maj, min, "GridContainer");
    qmlRegisterType<QQuickFormContainer>                 (uri, maj, min, "FormContainer");
    qmlRegisterType<QQuickStretchRowContainer>           (uri, maj, min, "StretchRowContainer");
    qmlRegisterType<QQuickStretchColumnContainer>        (uri, maj, min, "StretchColumnContainer");
    qmlRegisterType<QQuickWrapLeftRightContainer>        (uri, maj, min, "WrapLeftRightContainer");
    qmlRegisterType<QQuickWrapLeftRightContainerBreaker> (uri, maj, min, "WrapBreaker");

    qmlRegisterType<QQuickPixelPerfectContainer>         (uri, maj, min, "PixelPerfectContainer");

    qmlRegisterType<QQmlFileSystemModelEntry>            (uri, maj, min, "FileSystemModelEntry");

    qmlRegisterUncreatableType<QQuickExtraAnchors>       (uri, maj, min, "ExtraAnchors", "!!!");

    qmlRegisterSingletonType<QQmlFileSystemSingleton>    (uri, maj, min, "FileSystem",   &QQmlFileSystemSingleton::qmlSingletonProvider);
    qmlRegisterSingletonType<QQmlIntrospector>           (uri, maj, min, "Introspector", &QQmlIntrospector::qmlSingletonProvider);

    QQuickSvgIconHelper::setBasePath (":/QtQmlTricks/icons");

    if (engine != Q_NULLPTR) {
        engine->addImportPath ("qrc:///imports");
        engine->addImageProvider ("icon-theme", new QQuickThemeIconProvider);
    }
    else {
        qWarning () << "You didn't pass a QML engine to the register function,"
                    << "some features (mostly plain QML components, and icon theme provider) won't work !";
    }
}

#endif // QTQMLTRICKSPLUGIN_UIELEMENTS_H
