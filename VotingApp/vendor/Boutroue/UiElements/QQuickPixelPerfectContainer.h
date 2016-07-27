#ifndef QQUICKPIXELPERFECTCONTAINER_H
#define QQUICKPIXELPERFECTCONTAINER_H

#include <QQuickItem>
#include <QVector>
#include <QTimer>

class QQuickPixelPerfectContainer : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY (QQuickItem * contentItem READ getContentItem WRITE setContentItem NOTIFY contentItemChanged)

public:
    explicit QQuickPixelPerfectContainer (QQuickItem * parent = Q_NULLPTR);

    void classBegin        (void);
    void componentComplete (void);

    QQuickItem * getContentItem (void) const;

public slots:
    void setContentItem (QQuickItem * contentItem);

signals:
    void contentItemChanged (QQuickItem * contentItem);

protected:
    void updatePolish (void);

protected slots:
    void restartTimer (void);

private:
    QTimer m_inhibitTimer;
    QQuickItem * m_contentItem;
    QVector<QQuickItem *> m_ancestors;
};

#endif // QQUICKPIXELPERFECTCONTAINER_H
