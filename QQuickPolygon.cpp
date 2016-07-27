
#include "QQuickPolygon.h"

#include <qmath.h>

QQuickPolygon::QQuickPolygon (QQuickItem * parent)
    : QQuickItem (parent)
    , m_closed (true)
    , m_border (0.0)
    , m_minX   (0.0)
    , m_maxX   (0.0)
    , m_minY   (0.0)
    , m_maxY   (0.0)
    , m_color  (Qt::magenta)
    , m_stroke (Qt::transparent)
    , m_node         (Q_NULLPTR)
    , m_foreNode     (Q_NULLPTR)
    , m_backNode     (Q_NULLPTR)
    , m_foreGeometry (Q_NULLPTR)
    , m_backGeometry (Q_NULLPTR)
    , m_foreMaterial (Q_NULLPTR)
    , m_backMaterial (Q_NULLPTR)
{
    setFlag (QQuickItem::ItemHasContents);
}

QQuickPolygon::~QQuickPolygon (void) { }

qreal QQuickPolygon::getBorder (void) const {
    return m_border;
}

bool QQuickPolygon::getClosed (void) const {
    return m_closed;
}

QColor QQuickPolygon::getColor (void) const {
    return m_color;
}

QColor QQuickPolygon::getStroke (void) const {
    return m_stroke;
}

QVariantList QQuickPolygon::getPoints (void) const {
    QVariantList ret;
    foreach (const QPointF & point, m_points) {
        ret.append (point);
    }
    return ret;
}

void QQuickPolygon::setBorder (const qreal border) {
    if (m_border != border) {
        m_border = border;
        emit borderChanged ();
        update ();
    }
}

void QQuickPolygon::setClosed (const bool closed) {
    if (m_closed != closed) {
        m_closed = closed;
        emit closedChanged ();
        update ();
    }
}

void QQuickPolygon::setColor (const QColor & color) {
    if (m_color != color) {
        m_color = color;
        emit colorChanged ();
        update ();
    }
}

void QQuickPolygon::setStroke (const QColor & stroke) {
    if (m_stroke != stroke) {
        m_stroke = stroke;
        emit strokeChanged ();
        update ();
    }
}
void QQuickPolygon::setPoints (const QVariantList & points) {
    static const qreal BOUNDS = 999999999;
    bool dirty = false;
    const int count = points.size ();
    if (m_points.size () != count) {
        m_points.resize (count);
        dirty = true;
    }
    for (int idx = 0; idx < count; idx++) {
        const QPointF pt = points.at (idx).value<QPointF> ();
        if (pt != m_points.at (idx)) {
            if (pt.x () <= +BOUNDS &&
                pt.x () >= -BOUNDS &&
                pt.y () <= +BOUNDS &&
                pt.y () >= -BOUNDS) {
                m_points [idx] = pt;
                dirty = true;
            }
            else {
                m_points.clear();
                dirty = true;
                break;
            }
        }
    }
    if (dirty) {
        processTriangulation ();
        emit pointsChanged ();
        update ();
    }
}

static inline qreal getAngleFromSegment (const QPointF & startPoint, const QPointF & endPoint) {
    return qAtan2 (endPoint.y () - startPoint.y (), endPoint.x () - startPoint.x ());
}

void QQuickPolygon::cleanup (void) {
    if (m_backMaterial != Q_NULLPTR) {
        delete m_backMaterial;
        m_backMaterial = Q_NULLPTR;
    }
    if (m_foreMaterial != Q_NULLPTR) {
        delete m_foreMaterial;
        m_foreMaterial = Q_NULLPTR;
    }
    if (m_backGeometry != Q_NULLPTR) {
        delete m_backGeometry;
        m_backGeometry = Q_NULLPTR;
    }
    if (m_foreGeometry != Q_NULLPTR) {
        delete m_foreGeometry;
        m_foreGeometry = Q_NULLPTR;
    }
    if (m_backNode != Q_NULLPTR) {
        delete m_backNode;
        m_backNode = Q_NULLPTR;
    }
    if (m_foreNode != Q_NULLPTR) {
        delete m_foreNode;
        m_foreNode = Q_NULLPTR;
    }
    if (m_node != Q_NULLPTR) {
        delete m_node;
        m_node = Q_NULLPTR;
    }
}

QSGNode * QQuickPolygon::updatePaintNode (QSGNode * oldNode, UpdatePaintNodeData * updatePaintNodeData) {
    Q_UNUSED (oldNode)
    Q_UNUSED (updatePaintNodeData)
    // remove old nodes
    cleanup ();
    m_node = new QSGNode;
    // polygon background tesselation
    if (!m_triangles.isEmpty () && m_color.alpha () > 0) {
        m_backGeometry = new QSGGeometry (QSGGeometry::defaultAttributes_Point2D (), m_triangles.size ());
        m_backGeometry->setDrawingMode (GL_TRIANGLES);
        QSGGeometry::Point2D * vertex = m_backGeometry->vertexDataAsPoint2D ();
        const int size = m_triangles.size ();
        for (int idx = 0; idx < size; idx++) {
            vertex [idx].x = m_triangles [idx].x ();
            vertex [idx].y = m_triangles [idx].y ();
        }
        m_backMaterial = new QSGFlatColorMaterial;
        m_backMaterial->setColor (m_color);
        m_backNode = new QSGGeometryNode;
        m_backNode->setGeometry (m_backGeometry);
        m_backNode->setMaterial (m_backMaterial);
        m_node->appendChildNode (m_backNode);
    }
    // polyline stroke generation
    if (m_points.size () >= 2 && m_border > 0 && m_stroke.alpha () > 0) {
        const int pointsCount    = m_points.size ();
        const int linesCount     = (m_closed ? pointsCount : pointsCount -1);
        const int trianglesCount = (linesCount * 2);
        const int vertexCount    = (trianglesCount * 3);
        const qreal halfStroke = (qreal (m_border) * 0.5);
        QVector<QPointF> trianglesStroke;
        trianglesStroke.reserve (vertexCount);
        QPointF firstVec1, firstVec2, lastVec1, lastVec2;
        for (int startPointIdx = 0, endPointIdx = 1; endPointIdx < pointsCount; startPointIdx++, endPointIdx++) {
            const bool isFirst = (startPointIdx == 0);
            const bool isLast  = (endPointIdx   == pointsCount -1);
            const QPointF startPoint = m_points [startPointIdx];
            const QPointF endPoint   = m_points [endPointIdx];
            const qreal currAngle = getAngleFromSegment (startPoint, endPoint);
            const qreal prevAngle = (!isFirst ? getAngleFromSegment (startPoint, m_points [startPointIdx -1]) : (m_closed ? getAngleFromSegment (startPoint, m_points.last ()) : currAngle + M_PI));
            const qreal nextAngle = (!isLast  ? getAngleFromSegment (m_points [endPointIdx +1], endPoint)     : (m_closed ? getAngleFromSegment (m_points.first (), endPoint)  : currAngle + M_PI));
            const qreal startAngle = ((currAngle + prevAngle) * 0.5);
            const qreal endAngle   = ((currAngle + nextAngle) * 0.5);
            const QPointF startPolar = QPointF (qCos (startAngle), qSin (startAngle));
            const QPointF endPolar   = QPointF (qCos (endAngle),   qSin (endAngle));
            const QPointF startVec1 = (startPoint + startPolar * (halfStroke / qSin (startAngle - currAngle)));
            const QPointF startVec2 = (startPoint + startPolar * (halfStroke / qSin (currAngle  - startAngle)));
            const QPointF endVec1   = (endPoint   + endPolar   * (halfStroke / qSin (endAngle   - currAngle)));
            const QPointF endVec2   = (endPoint   + endPolar   * (halfStroke / qSin (currAngle  - endAngle)));
            trianglesStroke << startVec1 << startVec2 << endVec2;
            trianglesStroke << endVec1   << endVec2   << startVec1;
            if (m_closed) {
                if (isFirst) {
                    firstVec1 = startVec1;
                    firstVec2 = startVec2;
                }
                if (isLast) {
                    lastVec1 = endVec1;
                    lastVec2 = endVec2;
                }
            }
        }
        if (m_closed) {
            trianglesStroke << lastVec1  << lastVec2  << firstVec2;
            trianglesStroke << firstVec1 << firstVec2 << lastVec1;
        }
        m_foreGeometry = new QSGGeometry (QSGGeometry::defaultAttributes_Point2D (), trianglesStroke.size ());
        m_foreGeometry->setDrawingMode (GL_TRIANGLES);
        QSGGeometry::Point2D * vertex = m_foreGeometry->vertexDataAsPoint2D ();
        const int size = trianglesStroke.size ();
        for (int idx = 0; idx < size; idx++) {
            vertex [idx].x = trianglesStroke [idx].x ();
            vertex [idx].y = trianglesStroke [idx].y ();
        }
        m_foreMaterial = new QSGFlatColorMaterial;
        m_foreMaterial->setColor (m_stroke);
        m_foreNode = new QSGGeometryNode;
        m_foreNode->setGeometry (m_foreGeometry);
        m_foreNode->setMaterial (m_foreMaterial);
        m_node->appendChildNode (m_foreNode);
    }
    return m_node;
}

void QQuickPolygon::processTriangulation (void) {
    // allocate and initialize list of Vertices in polygon
    const int n = m_points.size ();
    m_triangles.clear ();
    m_triangles.reserve (n * 3);
    if (n >= 3) {
        QVector<int> index (n);
        for (int i = 0; i < n; i++) {
            index [i] = i;
        }
        // remove nv-2 Vertices, creating 1 triangle every time
        int nv = n;
        int count = (2 * nv); // error detection
        QPolygonF triangle (3);
        for (int v = (nv -1), u, w; nv > 2;) {
            // if we loop, it is probably a non-simple polygon
            count--;
            if (count > 0) {
                // three consecutive vertices in current polygon, <u,v,w>
                u = (v    < nv ? v    : 0); // previous
                v = (u +1 < nv ? u +1 : 0); // new v
                w = (v +1 < nv ? v +1 : 0); // next
                triangle [0] = m_points [index [u]];
                triangle [1] = m_points [index [v]];
                triangle [2] = m_points [index [w]];
                QPolygonF result = triangle.intersected (m_points);
                if (result.isClosed ()) {
                    result.removeLast ();
                }
                if (result == triangle) {
                    // output Triangle
                    m_triangles.append (m_points [index [u]]);
                    m_triangles.append (m_points [index [v]]);
                    m_triangles.append (m_points [index [w]]);
                    index.remove (v); // remove v from remaining polygon
                    nv--;
                    count = (2 * nv); // reset error detection counter
                }
            }
            else {
                // Triangulate: ERROR - probable bad polygon!
                break;
            }
        }
    }
}
