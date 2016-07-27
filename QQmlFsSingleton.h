#ifndef QQMLFSSINGLETON_H
#define QQMLFSSINGLETON_H

#include <QObject>
#include <QString>
#include <QQmlEngine>
#include <QJSEngine>
#include <QFileInfo>
#include <QMimeDatabase>

class QQmlFileSystemModelEntry : public QObject {
    Q_OBJECT

    Q_PROPERTY (QString url          READ getUrl          CONSTANT)
    Q_PROPERTY (QString name         READ getName         CONSTANT)
    Q_PROPERTY (QString path         READ getPath         CONSTANT)
    Q_PROPERTY (QString mimeType     READ getMimeType     CONSTANT)
    Q_PROPERTY (QString extension    READ getExtension    CONSTANT)
    Q_PROPERTY (QString lastModified READ getLastModified CONSTANT)

    Q_PROPERTY (bool    isDir        READ getIsDir        CONSTANT)
    Q_PROPERTY (bool    isFile       READ getIsFile       CONSTANT)
    Q_PROPERTY (bool    isLink       READ getIsLink       CONSTANT)

    Q_PROPERTY (int     size         READ getSize         CONSTANT)
    Q_PROPERTY (int     permissions  READ getPermission   CONSTANT)

public:
    explicit QQmlFileSystemModelEntry (const QFileInfo & info = QFileInfo (), QObject * parent = Q_NULLPTR);

    static QMimeDatabase MIME_DATABASE;
    static const QString DATETIME_FORMAT;

    const QString & getUrl          (void) const;
    const QString & getName         (void) const;
    const QString & getPath         (void) const;
    const QString & getMimeType     (void) const;
    const QString & getExtension    (void) const;
    const QString & getLastModified (void) const;

    bool getIsDir  (void) const;
    bool getIsFile (void) const;
    bool getIsLink (void) const;

    int getSize       (void) const;
    int getPermission (void) const;

private:
    const QString m_url;
    const QString m_name;
    const QString m_path;
    const QString m_mimeType;
    const QString m_extension;
    const QString m_lastModified;
    const bool m_isDir;
    const bool m_isFile;
    const bool m_isLink;
    const int m_size;
    const int m_permissions;
};

class QQmlFileSystemSingleton : public QObject {
    Q_OBJECT

    Q_ENUMS (Permission)

    Q_PROPERTY (QString homePath             READ getHomePath             CONSTANT)
    Q_PROPERTY (QString rootPath             READ getRootPath             CONSTANT)
    Q_PROPERTY (QString tempPath             READ getTempPath             CONSTANT)
    Q_PROPERTY (QString appDirPath           READ getAppDirPath           CONSTANT)
    Q_PROPERTY (QString appCachePath         READ getAppCachePath         CONSTANT)
    Q_PROPERTY (QString appConfigPath        READ getAppConfigPath        CONSTANT)
    Q_PROPERTY (QString documentsPath        READ getDocumentsPath        CONSTANT)
    Q_PROPERTY (QString imagesPath           READ getImagesPath           CONSTANT)
    Q_PROPERTY (QString musicPath            READ getMusicPath            CONSTANT)
    Q_PROPERTY (QString videosPath           READ getVideosPath           CONSTANT)
    Q_PROPERTY (QString downloadsPath        READ getDownloadsPath        CONSTANT)
    Q_PROPERTY (QString workingDirectoryPath READ getWorkingDirectoryPath CONSTANT)

    Q_PROPERTY (QStringList drivesList READ getDrivesList NOTIFY drivesListChanged)

public:
    static QObject * qmlSingletonProvider (QQmlEngine * qmlEngine, QJSEngine * jsEngine);

    enum Permission {
        ReadOwner = 0x4000, WriteOwner = 0x2000, ExeOwner = 0x1000,
        ReadUser  = 0x0400, WriteUser  = 0x0200, ExeUser  = 0x0100,
        ReadGroup = 0x0040, WriteGroup = 0x0020, ExeGroup = 0x0010,
        ReadOther = 0x0004, WriteOther = 0x0002, ExeOther = 0x0001,
    };

    const QString & getHomePath             (void) const;
    const QString & getRootPath             (void) const;
    const QString & getTempPath             (void) const;
    const QString & getAppDirPath           (void) const;
    const QString & getAppCachePath         (void) const;
    const QString & getAppConfigPath        (void) const;
    const QString & getDocumentsPath        (void) const;
    const QString & getImagesPath           (void) const;
    const QString & getMusicPath            (void) const;
    const QString & getVideosPath           (void) const;
    const QString & getDownloadsPath        (void) const;
    const QString & getWorkingDirectoryPath (void) const;

    const QStringList & getDrivesList (void) const;

public slots:
    bool isDir  (const QString & path) const;
    bool isFile (const QString & path) const;
    bool isLink (const QString & path) const;
    bool exists (const QString & path) const;

    bool copy (const QString & sourcePath, const QString & destPath) const;
    bool move (const QString & sourcePath, const QString & destPath) const;
    bool link (const QString & sourcePath, const QString & destPath) const;

    bool remove (const QString & path) const;

    int  size (const QString & path) const;

    QString parentDir (const QString & path) const;

    QString readTextFile  (const QString & path) const;
    bool    writeTextFile (const QString & path, const QString & text) const;

    QString pathFromUrl (const QUrl    & url)  const;
    QUrl    urlFromPath (const QString & path) const;

    QVariantList list (const QString & dirPath,
                       const QStringList & nameFilters = QStringList (),
                       const bool showHidden = false,
                       const bool showFiles = true) const;

signals:
    void drivesListChanged (const QStringList & drivesList);

protected slots:
    void doRefreshDrives (void);

private:
    const QString m_homePath;
    const QString m_rootPath;
    const QString m_tempPath;
    const QString m_appDirPath;
    const QString m_appCachePath;
    const QString m_appConfigPath;
    const QString m_documentsPath;
    const QString m_imagesPath;
    const QString m_musicPath;
    const QString m_videosPath;
    const QString m_downloadsPath;
    const QString m_workingDirectoryPath;
    QStringList m_drivesList;

    explicit QQmlFileSystemSingleton (QObject * parent = Q_NULLPTR);
};

#endif // QQMLFSSINGLETON_H
