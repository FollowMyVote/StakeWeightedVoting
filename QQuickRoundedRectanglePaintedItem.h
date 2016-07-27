#ifndef ROUNDEDRECTANGLEPAINTEDITEM_H
#define ROUNDEDRECTANGLEPAINTEDITEM_H

#include <QQuickPaintedItem>
#include <QPainter>
#include <QVector>
#include <QPointF>
#include <QColor>

class QQuickRoundedRectanglePaintedItem : public QQuickPaintedItem {
    Q_OBJECT

    Q_PROPERTY (QColor fillColor READ getFillColor WRITE setFillColor NOTIFY fillColorChanged) // white
    Q_PROPERTY (QColor lineColor READ getLineColor WRITE setLineColor NOTIFY lineColorChanged) // black

    Q_PROPERTY (int borderSize       READ getBorderSize       WRITE setBorderSize       NOTIFY borderSizeChanged) // 1
    Q_PROPERTY (int topBorderSize    READ getTopBorderSize    WRITE setTopBorderSize    NOTIFY topBorderSizeChanged) // -1
    Q_PROPERTY (int leftBorderSize   READ getLeftBorderSize   WRITE setLeftBorderSize   NOTIFY leftBorderSizeChanged) // -1
    Q_PROPERTY (int rightBorderSize  READ getRightBorderSize  WRITE setRightBorderSize  NOTIFY rightBorderSizeChanged) // -1
    Q_PROPERTY (int bottomBorderSize READ getBottomBorderSize WRITE setBottomBorderSize NOTIFY bottomBorderSizeChanged) // -1

    Q_PROPERTY (int radius            READ getRadius            WRITE setRadius            NOTIFY radiusChanged) // 0
    Q_PROPERTY (int topLeftRadius     READ getTopLeftRadius     WRITE setTopLeftRadius     NOTIFY topLeftRadiusChanged) // -1
    Q_PROPERTY (int topRightRadius    READ getTopRightRadius    WRITE setTopRightRadius    NOTIFY topRightRadiusChanged) // -1
    Q_PROPERTY (int bottomLeftRadius  READ getBottomLeftRadius  WRITE setBottomLeftRadius  NOTIFY bottomLeftRadiusChanged) // -1
    Q_PROPERTY (int bottomRightRadius READ getBottomRightRadius WRITE setBottomRightRadius NOTIFY bottomRightRadiusChanged) // -1

public:
    explicit QQuickRoundedRectanglePaintedItem (QQuickItem * parent = Q_NULLPTR);

    void paint (QPainter * painter);

    QColor getFillColor (void) const;
    QColor getLineColor (void) const;

    int getBorderSize       (void) const;
    int getTopBorderSize    (void) const;
    int getLeftBorderSize   (void) const;
    int getRightBorderSize  (void) const;
    int getBottomBorderSize (void) const;

    int getRadius            (void) const;
    int getTopLeftRadius     (void) const;
    int getTopRightRadius    (void) const;
    int getBottomLeftRadius  (void) const;
    int getBottomRightRadius (void) const;

public slots:
    void setFillColor (const QColor & fillColor);
    void setLineColor (const QColor & lineColor);

    void setBorderSize       (const int borderSize);
    void setTopBorderSize    (const int topBorderSize);
    void setLeftBorderSize   (const int leftBorderSize);
    void setRightBorderSize  (const int rightBorderSize);
    void setBottomBorderSize (const int bottomBorderSize);

    void setRadius            (const int radius);
    void setTopLeftRadius     (const int topLeftRadius);
    void setTopRightRadius    (const int topRightRadius);
    void setBottomLeftRadius  (const int bottomLeftRadius);
    void setBottomRightRadius (const int bottomRightRadius);

signals:
    void fillColorChanged         (void);
    void lineColorChanged         (void);
    void borderSizeChanged        (void);
    void topBorderSizeChanged     (void);
    void leftBorderSizeChanged    (void);
    void rightBorderSizeChanged   (void);
    void bottomBorderSizeChanged  (void);
    void radiusChanged            (void);
    void topLeftRadiusChanged     (void);
    void topRightRadiusChanged    (void);
    void bottomLeftRadiusChanged  (void);
    void bottomRightRadiusChanged (void);

protected:
    void recomputePointsList (void);

private:
    QColor m_fillColor;
    QColor m_lineColor;
    int m_borderSize;
    int m_topBorderSize;
    int m_leftBorderSize;
    int m_rightBorderSize;
    int m_bottomBorderSize;
    int m_radius;
    int m_topLeftRadius;
    int m_topRightRadius;
    int m_bottomLeftRadius;
    int m_bottomRightRadius;
    QVector<QPointF> m_pointsInner;
    QVector<QPointF> m_pointsOuter;
};

#endif // ROUNDEDRECTANGLEPAINTEDITEM_H
