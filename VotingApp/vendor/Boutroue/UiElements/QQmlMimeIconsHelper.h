#ifndef QQMLMIMEICONSHELPER_H
#define QQMLMIMEICONSHELPER_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QMimeDatabase>
#include <QMimeType>

class QQmlMimeIconsHelper : public QObject {
    Q_OBJECT

public:
    explicit QQmlMimeIconsHelper (QObject * parent = Q_NULLPTR);

public slots:
    QString getSvgIconPathForUrl   (const QString & url);
    QString getThemeIconNameForUrl (const QString & url);

private:
    class MetaDataCache {
    public:
        explicit MetaDataCache (void);

        void registerSpecialFolderIconForPath (const QString & path, const QString & icon);
        void registerSvgIconForMimeType       (const QString & type, const QString & icon);
        void registerThemeIconForMimeType     (const QString & type, const QString & icon);

        QString getSpecialFolderIconForPath (const QString & path);
        QString getSvgIconForMimeType       (const QString & type);
        QString getThemeIconForMimeType     (const QString & type);

        QMimeType getMimeTypeForFile (const QString & path);

    private:
        QMimeDatabase mimeDb;
        QHash<QString, QString> svgIconForMimetype;
        QHash<QString, QString> themeIconNameForMimeType;
        QHash<QString, QString> specialFoldersIconNames;
    };
    static MetaDataCache s_cache;
};

#endif // QQMLMIMEICONSHELPER_H
