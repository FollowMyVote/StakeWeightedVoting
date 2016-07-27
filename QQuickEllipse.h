#ifndef QQUICKELLIPSE_H
#define QQUICKELLIPSE_H

#include <QQuickItem>
#include <QColor>
#include <QVector>
#include <QPointF>
#include <QSGNode>
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>

class QQuickEllipse : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY (int holeWidth  READ getHoleWidth  WRITE setHoleWidth  NOTIFY holeWidthChanged) // 0
    Q_PROPERTY (int holeHeight READ getHoleHeight WRITE setHoleHeight NOTIFY holeHeightChanged) // 0
    Q_PROPERTY (int startAngle READ getStartAngle WRITE setStartAngle NOTIFY startAngleChanged) // 0
    Q_PROPERTY (int stopAngle  READ getStopAngle  WRITE setStopAngle  NOTIFY stopAngleChanged) // 0
    Q_PROPERTY (bool clockwise READ getClockwise WRITE setClockwise NOTIFY clockwiseChanged) // true
    Q_PROPERTY (QColor color READ getColor WRITE setColor NOTIFY colorChanged) // black

public:
    explicit QQuickEllipse (QQuickItem * parent = Q_NULLPTR);

    QSGNode * updatePaintNode (QSGNode * oldNode, UpdatePaintNodeData * updateData);

    int            getHoleWidth  (void) const;
    int            getHoleHeight (void) const;
    int            getStartAngle (void) const;
    int            getStopAngle  (void) const;
    bool           getClockwise  (void) const;
    const QColor & getColor      (void) const;

public slots:
    void setHoleWidth  (const int holeWidth);
    void setHoleHeight (const int holeHeight);
    void setStartAngle (const int startAngle);
    void setStopAngle  (const int stopAngle);
    void setClockwise  (const bool clockwise);
    void setColor      (const QColor & color);

signals:
    void holeWidthChanged  (void);
    void holeHeightChanged (void);
    void startAngleChanged (void);
    void stopAngleChanged  (void);
    void clockwiseChanged  (void);
    void colorChanged      (void);

protected:
    static QPointF trigoPoint (const int angleDeg);

private:
    int m_holeWidth;
    int m_holeHeight;
    int m_startAngle;
    int m_stopAngle;
    bool m_clockwise;
    QColor m_color;
};

#endif // QQUICKELLIPSE_H
