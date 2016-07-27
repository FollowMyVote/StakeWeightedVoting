#ifndef QQUICKEXTRAANCHORS_H
#define QQUICKEXTRAANCHORS_H

#include <QObject>
#include <QString>
#include <QQuickItem>

class QQuickExtraAnchors : public QObject {
    Q_OBJECT
    /// side dock
    Q_PROPERTY (QQuickItem * topDock           READ getTopDock           WRITE setTopDock           NOTIFY topDockChanged)
    Q_PROPERTY (QQuickItem * leftDock          READ getLeftDock          WRITE setLeftDock          NOTIFY leftDockChanged)
    Q_PROPERTY (QQuickItem * rightDock         READ getRightDock         WRITE setRightDock         NOTIFY rightDockChanged)
    Q_PROPERTY (QQuickItem * bottomDock        READ getBottomDock        WRITE setBottomDock        NOTIFY bottomDockChanged)
    /// axis fill
    Q_PROPERTY (QQuickItem * verticalFill      READ getVerticalFill      WRITE setVerticalFill      NOTIFY verticalFillChanged)
    Q_PROPERTY (QQuickItem * horizontalFill    READ getHorizontalFill    WRITE setHorizontalFill    NOTIFY horizontalFillChanged)
    /// corner position
    Q_PROPERTY (QQuickItem * topLeftCorner     READ getTopLeftCorner     WRITE setTopLeftCorner     NOTIFY topLeftCornerChanged)
    Q_PROPERTY (QQuickItem * topRightCorner    READ getTopRightCorner    WRITE setTopRightCorner    NOTIFY topRightCornerChanged)
    Q_PROPERTY (QQuickItem * bottomLeftCorner  READ getBottomLeftCorner  WRITE setBottomLeftCorner  NOTIFY bottomLeftCornerChanged)
    Q_PROPERTY (QQuickItem * bottomRightCorner READ getBottomRightCorner WRITE setBottomRightCorner NOTIFY bottomRightCornerChanged)

public:
    explicit QQuickExtraAnchors (QObject * parent = Q_NULLPTR);

    enum Sides {
        TOP,
        LEFT,
        RIGHT,
        BOTTOM,
    };

    static QQuickExtraAnchors * qmlAttachedProperties (QObject * object);

    QQuickItem * getTopDock           (void) const;
    QQuickItem * getLeftDock          (void) const;
    QQuickItem * getRightDock         (void) const;
    QQuickItem * getBottomDock        (void) const;
    QQuickItem * getVerticalFill      (void) const;
    QQuickItem * getHorizontalFill    (void) const;
    QQuickItem * getTopLeftCorner     (void) const;
    QQuickItem * getTopRightCorner    (void) const;
    QQuickItem * getBottomLeftCorner  (void) const;
    QQuickItem * getBottomRightCorner (void) const;

public slots:
    void setTopDock           (QQuickItem * topDock);
    void setLeftDock          (QQuickItem * leftDock);
    void setRightDock         (QQuickItem * rightDock);
    void setBottomDock        (QQuickItem * bottomDock);
    void setVerticalFill      (QQuickItem * verticalFill);
    void setHorizontalFill    (QQuickItem * horizontalFill);
    void setTopLeftCorner     (QQuickItem * topLeftCorner);
    void setTopRightCorner    (QQuickItem * topRightCorner);
    void setBottomLeftCorner  (QQuickItem * bottomLeftCorner);
    void setBottomRightCorner (QQuickItem * bottomRightCorner);

signals:
    void topDockChanged           (QQuickItem * topDock);
    void leftDockChanged          (QQuickItem * leftDock);
    void rightDockChanged         (QQuickItem * rightDock);
    void bottomDockChanged        (QQuickItem * bottomDock);
    void verticalFillChanged      (QQuickItem * verticalFill);
    void horizontalFillChanged    (QQuickItem * horizontalFill);
    void topLeftCornerChanged     (QQuickItem * topLeftCorner);
    void topRightCornerChanged    (QQuickItem * topRightCorner);
    void bottomLeftCornerChanged  (QQuickItem * bottomLeftCorner);
    void bottomRightCornerChanged (QQuickItem * bottomRightCorner);

protected:
    void defineAnchorLine (QQuickItem * other, const Sides side);

private:
    QObject    * m_anchors;
    QQuickItem * m_dockTop;
    QQuickItem * m_dockLeft;
    QQuickItem * m_dockRight;
    QQuickItem * m_dockBottom;
    QQuickItem * m_verticalFill;
    QQuickItem * m_horizontalFill;
    QQuickItem * m_topLeftCorner;
    QQuickItem * m_topRightCorner;
    QQuickItem * m_bottomLeftCorner;
    QQuickItem * m_bottomRightCorner;
};

QML_DECLARE_TYPE (QQuickExtraAnchors)
QML_DECLARE_TYPEINFO (QQuickExtraAnchors, QML_HAS_ATTACHED_PROPERTIES)

#endif // QQUICKEXTRAANCHORS_H
