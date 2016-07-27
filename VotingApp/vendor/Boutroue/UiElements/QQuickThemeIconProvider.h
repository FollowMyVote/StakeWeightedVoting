#ifndef QQUICKTHEMEICONPROVIDER_H
#define QQUICKTHEMEICONPROVIDER_H

#include <QObject>
#include <QQuickImageProvider>
#include <QPixmap>

class QQuickThemeIconProvider : public QQuickImageProvider {
  public:
      explicit QQuickThemeIconProvider (void);

      QPixmap requestPixmap (const QString & id,
                             QSize * actualSize,
                             const QSize & requestedSize);
};

#endif // QQUICKTHEMEICONPROVIDER_H
