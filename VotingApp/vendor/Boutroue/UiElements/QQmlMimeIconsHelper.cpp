
#include "QQmlMimeIconsHelper.h"

#include <QIcon>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUrl>

QQmlMimeIconsHelper::MetaDataCache QQmlMimeIconsHelper::s_cache;

QQmlMimeIconsHelper::QQmlMimeIconsHelper (QObject * parent) : QObject (parent) { }

QString QQmlMimeIconsHelper::getSvgIconPathForUrl (const QString & url) {
    QString ret;
    const QString path = QUrl (url).toLocalFile ();
    const QFileInfo info (path);
    if (info.exists ()) {
        if (info.isDir ()) {
            ret = s_cache.getSpecialFolderIconForPath (path);
            if (ret.isEmpty ()) {
                ret = "folder-closed";
            }
        }
        else {
            const QMimeType type = s_cache.getMimeTypeForFile (path);
            const QString tmp = type.name ();
            ret = s_cache.getSvgIconForMimeType (tmp);
            if (ret.isEmpty ()) {
                ret = "file";
                //qWarning () << "No icon for this MIME-type" << tmp;
            }
        }
    }
    return QStringLiteral ("qrc:///QtQmlTricks/icons/filetypes/%1.svg").arg (ret);
}

QString QQmlMimeIconsHelper::getThemeIconNameForUrl (const QString & url) {
    QString ret;
    const QString path = QUrl (url).toLocalFile ();
    const QFileInfo info (path);
    if (info.exists ()) {
        if (info.isDir ()) {
            ret = s_cache.getSpecialFolderIconForPath (path);
            if (ret.isEmpty ()) {
                ret = "folder";
            }
        }
        else {
            const QMimeType type = s_cache.getMimeTypeForFile (path);
            const QString tmp = type.name ();
            ret = s_cache.getThemeIconForMimeType (tmp);
            if (ret.isEmpty ()) {
                if (QIcon::hasThemeIcon (type.iconName ())) {
                    s_cache.registerThemeIconForMimeType (tmp, type.iconName ());
                }
                else if (QIcon::hasThemeIcon (type.genericIconName ())) {
                    s_cache.registerThemeIconForMimeType (tmp, type.genericIconName ());
                }
                else {
                    s_cache.registerThemeIconForMimeType (tmp, "empty");
                }
            }
            ret = s_cache.getThemeIconForMimeType (tmp);
        }
    }
    return ret;
}

void QQmlMimeIconsHelper::MetaDataCache::registerSpecialFolderIconForPath (const QString & path, const QString & icon) {
    specialFoldersIconNames.insert (path, icon);
}

void QQmlMimeIconsHelper::MetaDataCache::registerSvgIconForMimeType (const QString & type, const QString & icon) {
    svgIconForMimetype.insert (type, icon);
}

void QQmlMimeIconsHelper::MetaDataCache::registerThemeIconForMimeType (const QString & type, const QString & icon) {
    themeIconNameForMimeType.insert (type, icon);
}

QMimeType QQmlMimeIconsHelper::MetaDataCache::getMimeTypeForFile (const QString & path) {
    return mimeDb.mimeTypeForFile (path);
}

QString QQmlMimeIconsHelper::MetaDataCache::getSpecialFolderIconForPath (const QString & path) {
    return specialFoldersIconNames.value (path);
}

QString QQmlMimeIconsHelper::MetaDataCache::getSvgIconForMimeType (const QString & type) {
    return svgIconForMimetype.value (type);
}

QString QQmlMimeIconsHelper::MetaDataCache::getThemeIconForMimeType (const QString & type) {
    return themeIconNameForMimeType.value (type);
}

QQmlMimeIconsHelper::MetaDataCache::MetaDataCache (void) {
    svgIconForMimetype.reserve (100);
    specialFoldersIconNames.reserve (20);
    themeIconNameForMimeType.reserve (100);

    /// register special folders

    foreach (const QString & path, QStandardPaths::standardLocations (QStandardPaths::HomeLocation)) {
        registerSpecialFolderIconForPath (path, "folder-home");
    }
    foreach (const QString & path, QStandardPaths::standardLocations (QStandardPaths::DocumentsLocation)) {
        registerSpecialFolderIconForPath (path, "folder-documents");
    }
    foreach (const QString & path, QStandardPaths::standardLocations (QStandardPaths::MusicLocation)) {
        registerSpecialFolderIconForPath (path, "folder-music");
    }
    foreach (const QString & path, QStandardPaths::standardLocations (QStandardPaths::PicturesLocation)) {
        registerSpecialFolderIconForPath (path, "folder-images");
    }
    foreach (const QString & path, QStandardPaths::standardLocations (QStandardPaths::MoviesLocation)) {
        registerSpecialFolderIconForPath (path, "folder-videos");
    }
    foreach (const QString & path, QStandardPaths::standardLocations (QStandardPaths::DownloadLocation)) {
        registerSpecialFolderIconForPath (path, "folder-downloads");
    }

    /// register SVG icons for MIME-types

    registerSvgIconForMimeType ("image/png", "image");
    registerSvgIconForMimeType ("image/jpeg", "image");
    registerSvgIconForMimeType ("image/gif", "image");

    registerSvgIconForMimeType ("image/svg", "drawing");
    registerSvgIconForMimeType ("image/svg+xml", "drawing");
    registerSvgIconForMimeType ("application/vnd.oasis.opendocument.graphics", "drawing");

    registerSvgIconForMimeType ("audio/mpeg", "sound");
    registerSvgIconForMimeType ("audio/x-wav", "sound");
    registerSvgIconForMimeType ("audio/midi", "sound");

    registerSvgIconForMimeType ("video/mp4", "video");

    registerSvgIconForMimeType ("text/x-csrc", "code");
    registerSvgIconForMimeType ("text/x-chdr", "code");
    registerSvgIconForMimeType ("text/x-c++src", "code");
    registerSvgIconForMimeType ("text/x-c++hdr", "code");
    registerSvgIconForMimeType ("text/x-qml", "code");
    registerSvgIconForMimeType ("text/x-java", "code");
    registerSvgIconForMimeType ("text/css", "code");
    registerSvgIconForMimeType ("application/javascript", "code");

    registerSvgIconForMimeType ("application/xml", "xml");

    registerSvgIconForMimeType ("application/x-shellscript", "script");
    registerSvgIconForMimeType ("application/x-perl", "script");

    registerSvgIconForMimeType ("application/x-object", "binary");
    registerSvgIconForMimeType ("application/octet-stream", "binary");

    registerSvgIconForMimeType ("application/x-cd-image", "disk-image");

    registerSvgIconForMimeType ("application/zip", "archive");
    registerSvgIconForMimeType ("application/x-xz-compressed-tar", "archive");
    registerSvgIconForMimeType ("application/x-compressed-tar", "archive");
    registerSvgIconForMimeType ("application/x-rar", "archive");
    registerSvgIconForMimeType ("application/x-rpm", "archive");
    registerSvgIconForMimeType ("application/gzip", "archive");
    registerSvgIconForMimeType ("application/vnd.debian.binary-package", "archive");
    registerSvgIconForMimeType ("application/vnd.android.package-archive", "archive");
    registerSvgIconForMimeType ("application/x-7z-compressed", "archive");
    registerSvgIconForMimeType ("application/x-bzip-compressed-tar", "archive");

    registerSvgIconForMimeType ("text/x-makefile", "text");
    registerSvgIconForMimeType ("text/x-log", "text");
    registerSvgIconForMimeType ("text/x-theme", "text");
    registerSvgIconForMimeType ("text/csv", "text");
    registerSvgIconForMimeType ("text/plain", "text");
    registerSvgIconForMimeType ("text/vcard", "text");
    registerSvgIconForMimeType ("text/markdown", "text");
    registerSvgIconForMimeType ("application/json", "text");

    registerSvgIconForMimeType ("application/pdf", "pdf");

    registerSvgIconForMimeType ("application/vnd.oasis.opendocument.text", "document");
    registerSvgIconForMimeType ("application/vnd.openxmlformats-officedocument.wordprocessingml.document", "document");
    registerSvgIconForMimeType ("application/msword", "document");

    registerSvgIconForMimeType ("application/vnd.oasis.opendocument.spreadsheet", "spreadsheet");
    registerSvgIconForMimeType ("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", "spreadsheet");
    registerSvgIconForMimeType ("application/vnd.ms-excel", "spreadsheet");
    registerSvgIconForMimeType ("application/ms-excel", "spreadsheet");

    registerSvgIconForMimeType ("application/vnd.oasis.opendocument.presentation", "slideshow");
    registerSvgIconForMimeType ("application/vnd.openxmlformats-officedocument.presentationml.presentation", "slideshow");
    registerSvgIconForMimeType ("application/vnd.ms-powerpoint", "slideshow");

    registerSvgIconForMimeType ("text/html", "webpage");

    registerSvgIconForMimeType ("application/sql", "database");
    registerSvgIconForMimeType ("application/x-sqlite3", "database");

    registerSvgIconForMimeType ("application/x-executable", "executable");
    registerSvgIconForMimeType ("application/x-ms-dos-executable", "executable");
}
