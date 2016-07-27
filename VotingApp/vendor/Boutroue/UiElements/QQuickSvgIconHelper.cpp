
#include "QQuickSvgIconHelper.h"

#include <QUrl>
#include <QDir>
#include <QHash>
#include <QFile>
#include <QImage>
#include <QDebug>
#include <QPainter>
#include <QDirIterator>
#include <QStringBuilder>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QSvgRenderer>
#include <QMultiHash>
#include <QByteArray>
#include <QSocketNotifier>
#include <QDebug>
#include <QVarLengthArray>

#ifdef Q_OS_LINUX
#   include <stdio.h>
#   include <errno.h>
#   include <unistd.h>
#   include <iostream>
#   include <sys/ioctl.h>
#   include <sys/inotify.h>
#endif

class SvgMetaDataCache : public QObject {
public:
    explicit SvgMetaDataCache (QObject * parent = Q_NULLPTR)
        : QObject (parent)
        , inotifyFd (-1)
        , inotifyWatcher (Q_NULLPTR)
        , hasher (QCryptographicHash::Md5)
    {
        changeBasePath  (qApp->applicationDirPath ());
        changeCachePath (QStandardPaths::writableLocation (QStandardPaths::CacheLocation) % "/SvgIconsCache");
#ifdef Q_OS_LINUX
        inotifyFd = inotify_init ();
        inotifyWatcher = new QSocketNotifier (inotifyFd, QSocketNotifier::Read);
        connect (inotifyWatcher, &QSocketNotifier::activated, this, &SvgMetaDataCache::onInotifyEvent);
#endif
    }

    virtual ~SvgMetaDataCache (void) {
#ifdef Q_OS_LINUX
        for (QHash<QString, int>::const_iterator it = descriptorsIndex.constBegin (); it != descriptorsIndex.constEnd (); it++) {
            inotify_rm_watch (inotifyFd, it.value ());
        }
        ::close (inotifyFd);
#endif
    }

    void changeBasePath (const QString & path) {
        basePath = path;
    }

    void changeCachePath (const QString & path) {
        cachePath = path;
        QDir ().mkpath (cachePath);
    }

    QString baseFile (const QString & name = "") {
        return (basePath % "/" % name);
    }

    QString cacheFile (const QString & name = "") {
        return (cachePath % "/" % name);
    }

    QString hashFile (const QString & filePath) {
        QString ret;
        QFile file (filePath);
        if (file.open (QFile::ReadOnly)) {
            hasher.reset ();
            hasher.addData (&file);
            ret = hasher.result ().toHex ();
            hasher.reset ();
            file.close ();
        }
        return ret;
    }

    QString hashData (const QByteArray & data) {
        QString ret;
        if (!data.isEmpty ()) {
            hasher.reset ();
            hasher.addData (data);
            ret = hasher.result ().toHex ();
            hasher.reset ();
        }
        return ret;
    }

    bool hasHashInIndex (const QString & hash) {
        return checksumsIndex.contains (hash);
    }

    void addHashInIndex (const QString & hash, const QString & checksum) {
        checksumsIndex.insert (hash, checksum);
    }

    void removeHashFromIndex (const QString & hash) {
        checksumsIndex.remove (hash);
    }

    QString readChecksumFile (const QString & filePath) {
        QString ret;
        QFile file (filePath);
        if (file.open (QFile::ReadOnly)) {
            ret = QString::fromLatin1 (file.readAll ());
            file.close ();
        }
        return ret;
    }

    void writeChecksumFile (const QString & filePath, const QString & checksum) {
        QFile file (filePath);
        if (file.open (QFile::WriteOnly)) {
            file.write (checksum.toLatin1 ());
            file.flush ();
            file.close ();
        }
    }

    bool renderSvgToPng (const QString & svgPath, const QString & pngPath, const QSize & size, const QColor & colorize) {
        bool ret = false;
        QImage image (size.width (), size.height (), QImage::Format_ARGB32);
        QPainter painter (&image);
        image.fill (Qt::transparent);
        painter.setRenderHint (QPainter::Antialiasing,            true);
        painter.setRenderHint (QPainter::SmoothPixmapTransform,   true);
        painter.setRenderHint (QPainter::HighQualityAntialiasing, true);
        renderer.load (svgPath);
        if (renderer.isValid ()) {
            renderer.render (&painter);
            if (colorize.isValid () && colorize.alpha () > 0) {
                QColor tmp (colorize);
                for (int x (0); x < image.width (); x++) {
                    for (int y (0); y < image.height (); y++) {
                        tmp.setAlpha (qAlpha (image.pixel (x, y)));
                        image.setPixel (x, y, tmp.rgba ());
                    }
                }
            }
            ret = image.save (pngPath, "PNG", 0);
        }
        return ret;
    }

    void addFileWatcher (const QString & path, QQuickSvgIconHelper * svgHelper) {
        if (!path.isEmpty () && !path.startsWith (":/") && !path.startsWith ("qrc:/") && svgHelper != Q_NULLPTR) {
#ifdef Q_OS_LINUX
            const QByteArray tmp = path.toLatin1 ();
            if (!descriptorsIndex.contains (path)) {
                const int desc = inotify_add_watch (inotifyFd, tmp.constData (), IN_MODIFY | IN_DELETE_SELF);
                descriptorsIndex.insert (path, desc);
            }
            if (!svgHelpersIndex.contains (path, svgHelper)) {
                svgHelpersIndex.insert (path, svgHelper);
            }
#endif
        }
    }

    void removeFileWatcher (const QString & path, QQuickSvgIconHelper * svgHelper) {
        if (!path.isEmpty () && !path.startsWith (":/") && !path.startsWith ("qrc:/") && svgHelper != Q_NULLPTR) {
#ifdef Q_OS_LINUX
            svgHelpersIndex.remove (path, svgHelper);
            if (svgHelpersIndex.values (path).isEmpty ()) {
                const int desc = descriptorsIndex.value (path, -1);
                if (desc > -1) {
                    inotify_rm_watch (inotifyFd, desc);
                    descriptorsIndex.remove (path);
                }
            }
#endif
        }
    }

protected:
    void onInotifyEvent (void) {
#ifdef Q_OS_LINUX
        static const int SIZEOF_EVT = sizeof (inotify_event);
        ssize_t buffSize = 0;
        ::ioctl (inotifyFd, FIONREAD, &buffSize);
        QByteArray buf (static_cast<int> (buffSize), '\0');
        buffSize = ::read (inotifyFd, buf.data (), static_cast<size_t> (buffSize));
        inotifyBuffer.append (buf.left (static_cast<int> (buffSize)));
        while (static_cast<int> (inotifyBuffer.size ()) >= SIZEOF_EVT) {
            const inotify_event * evt = reinterpret_cast<const inotify_event *> (inotifyBuffer.constData ());
            const QString path = descriptorsIndex.key (evt->wd);
            if (!path.isEmpty ()) {
                const QList<QQuickSvgIconHelper *> watchersList = svgHelpersIndex.values (path);
                for (QList<QQuickSvgIconHelper *>::const_iterator it = watchersList.constBegin (); it != watchersList.constEnd (); it++) {
                    QQuickSvgIconHelper * svgHelper = (* it);
                    if (svgHelper != Q_NULLPTR) {
                        if (evt->mask & IN_MODIFY) {
                            emit svgHelper->doForceRegen ();
                        }
                        else if (evt->mask & IN_DELETE_SELF) {
                            emit svgHelper->doForceRegen ();
                        }
                        else { }
                    }
                }
            }
            inotifyBuffer.remove (0, static_cast<int> (SIZEOF_EVT + evt->len));
        }
#endif
    }

private:
    int inotifyFd;
    QString basePath;
    QString cachePath;
    QByteArray inotifyBuffer;
    QSvgRenderer renderer;
    QSocketNotifier * inotifyWatcher;
    QCryptographicHash hasher;
    QHash<QString, int> descriptorsIndex;
    QHash<QString, QString> checksumsIndex;
    QMultiHash<QString, QQuickSvgIconHelper *> svgHelpersIndex;
};

QQuickSvgIconHelper::QQuickSvgIconHelper (QObject * parent)
    : QObject           (parent)
    , m_size            (0)
    , m_live            (false)
    , m_ready           (false)
    , m_verticalRatio   (1.0)
    , m_horizontalRatio (1.0)
    , m_color           (Qt::transparent)
    , m_icon            (QString ())
    , m_inhibitTimer    (this)
{
    m_inhibitTimer.setInterval (50);
    m_inhibitTimer.setSingleShot (true);
    connect (&m_inhibitTimer, &QTimer::timeout, this, &QQuickSvgIconHelper::doProcessIcon, Qt::UniqueConnection);
}

QQuickSvgIconHelper::~QQuickSvgIconHelper () {
    cache ().removeFileWatcher (m_sourcePath, this);
}

SvgMetaDataCache & QQuickSvgIconHelper::cache (void) {
    static SvgMetaDataCache ret;
    return ret;
}

void QQuickSvgIconHelper::classBegin (void) {
    m_ready = false;
}

void QQuickSvgIconHelper::componentComplete (void) {
    m_ready = true;
    scheduleRefresh ();
}

void QQuickSvgIconHelper::setTarget (const QQmlProperty & target) {
    m_property = target;
    scheduleRefresh ();
}

void QQuickSvgIconHelper::setBasePath (const QString & basePath) {
    cache ().changeBasePath (basePath);
}

void QQuickSvgIconHelper::setCachePath (const QString & cachePath) {
    cache ().changeCachePath (cachePath);
}

int QQuickSvgIconHelper::getSize (void) const {
    return m_size;
}

bool QQuickSvgIconHelper::getLive (void) const {
    return m_live;
}

qreal QQuickSvgIconHelper::getVerticalRatio (void) const {
    return m_verticalRatio;
}

qreal QQuickSvgIconHelper::getHorizontalRatio (void) const {
    return m_horizontalRatio;
}

const QColor & QQuickSvgIconHelper::getColor (void) const {
    return m_color;
}

const QString & QQuickSvgIconHelper::getIcon (void) const {
    return m_icon;
}

void QQuickSvgIconHelper::setSize (const int size) {
    if (m_size != size) {
        m_size = size;
        scheduleRefresh ();
        emit sizeChanged ();
    }
}

void QQuickSvgIconHelper::setVerticalRatio (const qreal ratio) {
    if (m_verticalRatio != ratio) {
        m_verticalRatio = ratio;
        scheduleRefresh ();
        emit verticalRatioChanged ();
    }
}

void QQuickSvgIconHelper::setHorizontalRatio (const qreal ratio) {
    if (m_horizontalRatio != ratio) {
        m_horizontalRatio = ratio;
        scheduleRefresh ();
        emit horizontalRatioChanged ();
    }
}

void QQuickSvgIconHelper::setColor (const QColor & color) {
    if (m_color != color) {
        m_color = color;
        scheduleRefresh ();
        emit colorChanged ();
    }
}

void QQuickSvgIconHelper::setIcon (const QString & icon) {
    if (m_icon != icon) {
        m_icon = icon;
        scheduleRefresh ();
        emit iconChanged ();
    }
}

void QQuickSvgIconHelper::scheduleRefresh (void) {
    m_inhibitTimer.stop ();
    m_inhibitTimer.start ();
}

void QQuickSvgIconHelper::doForceRegen (void) {
    if (!m_hash.isEmpty ()) {
        cache ().removeHashFromIndex (m_hash);
        scheduleRefresh ();
    }
}

void QQuickSvgIconHelper::doProcessIcon (void) {
    if (m_ready) {
        QUrl url;
        if (!m_icon.isEmpty () && m_size > 0 && m_horizontalRatio > 0.0 && m_verticalRatio > 0.0) {
            const QSize imgSize (int (m_size * m_horizontalRatio),
                                 int (m_size * m_verticalRatio));
            const QString sourcePath = (m_icon.startsWith ("file://")
                                        ? QUrl (m_icon).toLocalFile ()
                                        : (m_icon.startsWith ("qrc:/")
                                           ? QString (m_icon).replace (QRegularExpression ("qrc:/+"), ":/")
                                           : cache ().baseFile (m_icon % ".svg")));
            if (m_sourcePath != sourcePath) {
                m_sourcePath = sourcePath;
                const bool live = !sourcePath.startsWith (':');
                if (m_live != live) {
                    m_live = live;
                    emit liveChanged ();
                }
                cache ().addFileWatcher (m_sourcePath, this);
            }
            if (QFile::exists (m_sourcePath)) {
                m_hash = cache ().hashData (m_sourcePath.toLatin1 ());
                m_cachedPath = cache ().cacheFile (m_hash
                                                   % "_" % QString::number (imgSize.width ())
                                                   % "x" % QString::number (imgSize.height ())
                                                   % (m_color.alpha () > 0 ? m_color.name () : "")
                                                   % ".png");
                if (!cache ().hasHashInIndex (m_hash)) {
                    const QString checkumPath = cache ().cacheFile (m_hash % ".md5");
                    const QString reference = cache ().readChecksumFile (checkumPath);
                    const QString checksum  = cache ().hashFile (m_sourcePath);
                    if (reference != checksum) {
                        QDirIterator it (cache ().cacheFile (),
                                         QStringList (m_hash % "*.png"),
                                         QDir::Filters (QDir::Files | QDir::NoDotAndDotDot),
                                         QDirIterator::IteratorFlags (QDirIterator::NoIteratorFlags));
                        while (it.hasNext ()) {
                            QFile::remove (it.next ());
                        }
                        cache ().writeChecksumFile (checkumPath, checksum);
                    }
                    cache ().addHashInIndex (m_hash, checksum);
                }
                if (!QFile::exists (m_cachedPath)) {
                    cache ().renderSvgToPng (m_sourcePath, m_cachedPath, imgSize, m_color);
                }
                if (QFile::exists (m_cachedPath)) {
                    url = QUrl::fromLocalFile (m_cachedPath);
                }
            }
            else {
                qWarning () << ">>> QmlSvgIconHelper : Can't render" << m_sourcePath << ", no such file !";
            }
        }
        if (m_property.isValid () && m_property.isWritable ()) {
            m_property.write (QUrl ());
            m_property.write (url);
        }
    }
}
