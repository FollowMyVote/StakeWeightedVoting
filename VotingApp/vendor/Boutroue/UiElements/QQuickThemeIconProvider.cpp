
#include "QQuickThemeIconProvider.h"

#include <QIcon>
#include <QDebug>

QQuickThemeIconProvider::QQuickThemeIconProvider (void)
    : QQuickImageProvider (QQuickImageProvider::Pixmap)
{ }

QPixmap QQuickThemeIconProvider::requestPixmap (const QString & id, QSize * actualSize, const QSize & requestedSize) {
    static const QPixmap EMPTY_PIX = QPixmap ();
    static const QSize DEFAULT_SIZE = QSize (128, 128);
    QPixmap ret = EMPTY_PIX;
    const QString name = id;
    const QIcon icon = QIcon::fromTheme (name);
    if (!icon.isNull ()) {
        ret = icon.pixmap (requestedSize.isValid () ? requestedSize : DEFAULT_SIZE);
        if (!ret.isNull ()) {
            if (actualSize != Q_NULLPTR) {
                (* actualSize) = ret.size ();
            }
        }
    }
    else {
        qWarning () << "No icon named" << name << "in theme !";
    }
    return ret;
}
