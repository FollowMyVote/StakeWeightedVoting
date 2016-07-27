
#include "QQuickRoundedRectanglePaintedItem.h"

#include <QtMath>

static const QPointF & trigo (const int angle) {
    static bool ready = false;
    static QPointF cache [360];
    if (!ready) {
        for (int idx = 0; idx < 360; idx++) {
            cache [idx].setX (qCos (idx * M_PI / 180.0));
            cache [idx].setY (qSin (idx * M_PI / 180.0));
        }
        ready = true;
    }
    return cache [(360 + angle) % 360];
}

static inline QPointF multiplyPoints (const QPointF & p1, const QPointF & p2) {
    return QPointF (p1.x () * p2.x (), p1.y () * p2.y ());
}

static inline void addPointsFromArc (QVector<QPointF> & list, const QPointF & center, const QPointF & radius, const int startAngle, const int endAngle) {
    int angle = startAngle;
    while (angle != endAngle) {
        list.append (center + multiplyPoints (radius, trigo (angle)));
        angle = ((angle +1) % 360);
    }
}

QQuickRoundedRectanglePaintedItem::QQuickRoundedRectanglePaintedItem (QQuickItem * parent) : QQuickPaintedItem (parent)
  , m_fillColor         (Qt::white)
  , m_lineColor         (Qt::black)
  , m_borderSize        ( 1)
  , m_topBorderSize     (-1)
  , m_leftBorderSize    (-1)
  , m_rightBorderSize   (-1)
  , m_bottomBorderSize  (-1)
  , m_radius            ( 0)
  , m_topLeftRadius     (-1)
  , m_topRightRadius    (-1)
  , m_bottomLeftRadius  (-1)
  , m_bottomRightRadius (-1)
{
    connect (this, &QQuickRoundedRectanglePaintedItem::fillColorChanged,         this, &QQuickItem::update);
    connect (this, &QQuickRoundedRectanglePaintedItem::lineColorChanged,         this, &QQuickItem::update);
    connect (this, &QQuickRoundedRectanglePaintedItem::borderSizeChanged,        this, &QQuickItem::update);
    connect (this, &QQuickRoundedRectanglePaintedItem::topBorderSizeChanged,     this, &QQuickItem::update);
    connect (this, &QQuickRoundedRectanglePaintedItem::leftBorderSizeChanged,    this, &QQuickItem::update);
    connect (this, &QQuickRoundedRectanglePaintedItem::rightBorderSizeChanged,   this, &QQuickItem::update);
    connect (this, &QQuickRoundedRectanglePaintedItem::bottomBorderSizeChanged,  this, &QQuickItem::update);
    connect (this, &QQuickRoundedRectanglePaintedItem::radiusChanged,            this, &QQuickItem::update);
    connect (this, &QQuickRoundedRectanglePaintedItem::topLeftRadiusChanged,     this, &QQuickItem::update);
    connect (this, &QQuickRoundedRectanglePaintedItem::topRightRadiusChanged,    this, &QQuickItem::update);
    connect (this, &QQuickRoundedRectanglePaintedItem::bottomLeftRadiusChanged,  this, &QQuickItem::update);
    connect (this, &QQuickRoundedRectanglePaintedItem::bottomRightRadiusChanged, this, &QQuickItem::update);
}

void QQuickRoundedRectanglePaintedItem::recomputePointsList (void) {
    const qreal minX = 0;
    const qreal minY = 0;
    const qreal maxX = width ();
    const qreal maxY = height ();
    const int topLeftRadius     = (m_topLeftRadius >= 0     ? m_topLeftRadius     : m_radius);
    const int topRightRadius    = (m_topRightRadius >= 0    ? m_topRightRadius    : m_radius);
    const int bottomLeftRadius  = (m_bottomLeftRadius >= 0  ? m_bottomLeftRadius  : m_radius);
    const int bottomRightRadius = (m_bottomRightRadius >= 0 ? m_bottomRightRadius : m_radius);
    const int topBorderSize     = (m_topBorderSize >= 0     ? m_topBorderSize     : m_borderSize);
    const int leftBorderSize    = (m_leftBorderSize >= 0    ? m_leftBorderSize    : m_borderSize);
    const int rightBorderSize   = (m_rightBorderSize >= 0   ? m_rightBorderSize   : m_borderSize);
    const int bottomBorderSize  = (m_bottomBorderSize >= 0  ? m_bottomBorderSize  : m_borderSize);
    m_pointsOuter.clear ();
    m_pointsInner.clear ();
    m_pointsOuter.reserve (364);
    m_pointsInner.reserve (364);
    /// top left corner
    if (topLeftRadius) {
        addPointsFromArc (m_pointsOuter,
                          QPointF (topLeftRadius, topLeftRadius),
                          QPointF (topLeftRadius, topLeftRadius),
                          180, 270);
        addPointsFromArc (m_pointsInner,
                          QPointF (topLeftRadius, topLeftRadius),
                          QPointF (topLeftRadius - leftBorderSize, topLeftRadius - topBorderSize),
                          180, 270);
    }
    else {
        m_pointsOuter.append (QPointF (minX, minY));
        m_pointsInner.append (QPointF (leftBorderSize, topBorderSize));
    }
    /// top right corner
    if (topRightRadius) {
        addPointsFromArc (m_pointsOuter,
                          QPointF (maxX - topRightRadius, topRightRadius),
                          QPointF (topRightRadius, topRightRadius),
                          270, 0);
        addPointsFromArc (m_pointsInner,
                          QPointF (maxX - topRightRadius, topRightRadius),
                          QPointF (topRightRadius - rightBorderSize, topRightRadius - topBorderSize),
                          270, 0);
    }
    else {
        m_pointsOuter.append (QPointF (maxX, minY));
        m_pointsInner.append (QPointF (maxX - rightBorderSize, topBorderSize));
    }
    /// bottom right corner
    if (topRightRadius) {
        addPointsFromArc (m_pointsOuter,
                          QPointF (maxX - bottomRightRadius, maxY - bottomRightRadius),
                          QPointF (bottomRightRadius, bottomRightRadius),
                          0, 90);
        addPointsFromArc (m_pointsInner,
                          QPointF (maxX - bottomRightRadius, maxY - bottomRightRadius),
                          QPointF (bottomRightRadius - rightBorderSize, bottomRightRadius - bottomBorderSize),
                          0, 90);
    }
    else {
        m_pointsOuter.append (QPointF (maxX, maxY));
        m_pointsInner.append (QPointF (maxX - rightBorderSize, maxY - bottomBorderSize));
    }
    /// bottom left corner
    if (bottomLeftRadius) {
        addPointsFromArc (m_pointsOuter,
                          QPointF (bottomLeftRadius, maxY - bottomLeftRadius),
                          QPointF (bottomLeftRadius, bottomLeftRadius),
                          90, 180);
        addPointsFromArc (m_pointsInner,
                          QPointF (bottomLeftRadius, maxY - bottomLeftRadius),
                          QPointF (bottomLeftRadius - leftBorderSize, bottomLeftRadius - bottomBorderSize),
                          90, 180);
    }
    else {
        m_pointsOuter.append (QPointF (minX, maxY));
        m_pointsInner.append (QPointF (leftBorderSize, maxY - bottomBorderSize));
    }
}

void QQuickRoundedRectanglePaintedItem::paint (QPainter * painter) {
    recomputePointsList ();
    painter->setPen (Qt::NoPen);
    painter->setRenderHint (QPainter::Antialiasing);
    if (m_fillColor.alpha () > 0) {
        painter->setBrush (m_fillColor);
        painter->drawPolygon (QPolygonF (m_pointsInner));
    }
    if (m_lineColor.alpha () > 0) {
        painter->setBrush (m_lineColor);
        painter->drawPolygon (QPolygonF (m_pointsOuter).subtracted (m_pointsInner));
    }
}

QColor QQuickRoundedRectanglePaintedItem::getFillColor (void) const {
    return m_fillColor;
}

QColor QQuickRoundedRectanglePaintedItem::getLineColor (void) const {
    return m_lineColor;
}

int QQuickRoundedRectanglePaintedItem::getBorderSize (void) const {
    return m_borderSize;
}

int QQuickRoundedRectanglePaintedItem::getTopBorderSize (void) const {
    return m_topBorderSize;
}

int QQuickRoundedRectanglePaintedItem::getLeftBorderSize (void) const {
    return m_leftBorderSize;
}

int QQuickRoundedRectanglePaintedItem::getRightBorderSize (void) const {
    return m_rightBorderSize;
}

int QQuickRoundedRectanglePaintedItem::getBottomBorderSize (void) const {
    return m_bottomBorderSize;
}

int QQuickRoundedRectanglePaintedItem::getRadius (void) const {
    return m_radius;
}

int QQuickRoundedRectanglePaintedItem::getTopLeftRadius (void) const {
    return m_topLeftRadius;
}

int QQuickRoundedRectanglePaintedItem::getTopRightRadius (void) const {
    return m_topRightRadius;
}

int QQuickRoundedRectanglePaintedItem::getBottomLeftRadius (void) const {
    return m_bottomLeftRadius;
}

int QQuickRoundedRectanglePaintedItem::getBottomRightRadius (void) const {
    return m_bottomRightRadius;
}

void QQuickRoundedRectanglePaintedItem::setFillColor (const QColor &  fillColor) {
    if (m_fillColor != fillColor) {
        m_fillColor = fillColor;
        emit fillColorChanged ();
    }
}

void QQuickRoundedRectanglePaintedItem::setLineColor (const QColor &  lineColor) {
    if (m_lineColor != lineColor) {
        m_lineColor = lineColor;
        emit lineColorChanged ();
    }
}

void QQuickRoundedRectanglePaintedItem::setBorderSize (const int borderSize) {
    if (m_borderSize != borderSize) {
        m_borderSize = borderSize;
        emit borderSizeChanged ();
    }
}

void QQuickRoundedRectanglePaintedItem::setTopBorderSize (const int topBorderSize) {
    if (m_topBorderSize != topBorderSize) {
        m_topBorderSize = topBorderSize;
        emit topBorderSizeChanged ();
    }
}

void QQuickRoundedRectanglePaintedItem::setLeftBorderSize (const int leftBorderSize) {
    if (m_leftBorderSize != leftBorderSize) {
        m_leftBorderSize = leftBorderSize;
        emit leftBorderSizeChanged ();
    }
}

void QQuickRoundedRectanglePaintedItem::setRightBorderSize (const int rightBorderSize) {
    if (m_rightBorderSize != rightBorderSize) {
        m_rightBorderSize = rightBorderSize;
        emit rightBorderSizeChanged ();
    }
}

void QQuickRoundedRectanglePaintedItem::setBottomBorderSize (const int bottomBorderSize) {
    if (m_bottomBorderSize != bottomBorderSize) {
        m_bottomBorderSize = bottomBorderSize;
        emit bottomBorderSizeChanged ();
    }
}

void QQuickRoundedRectanglePaintedItem::setRadius (const int radius) {
    if (m_radius != radius) {
        m_radius = radius;
        emit radiusChanged ();
    }
}

void QQuickRoundedRectanglePaintedItem::setTopLeftRadius (const int topLeftRadius) {
    if (m_topLeftRadius != topLeftRadius) {
        m_topLeftRadius = topLeftRadius;
        emit topLeftRadiusChanged ();
    }
}

void QQuickRoundedRectanglePaintedItem::setTopRightRadius (const int topRightRadius) {
    if (m_topRightRadius != topRightRadius) {
        m_topRightRadius = topRightRadius;
        emit topRightRadiusChanged ();
    }
}

void QQuickRoundedRectanglePaintedItem::setBottomLeftRadius (const int bottomLeftRadius) {
    if (m_bottomLeftRadius != bottomLeftRadius) {
        m_bottomLeftRadius = bottomLeftRadius;
        emit bottomLeftRadiusChanged ();
    }
}

void QQuickRoundedRectanglePaintedItem::setBottomRightRadius (const int bottomRightRadius) {
    if (m_bottomRightRadius != bottomRightRadius) {
        m_bottomRightRadius = bottomRightRadius;
        emit bottomRightRadiusChanged ();
    }
}
