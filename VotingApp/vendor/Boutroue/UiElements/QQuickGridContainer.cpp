
#include "QQuickGridContainer.h"

#include <QtMath>
#include <QDebug>
#include <QElapsedTimer>

QQuickGridContainer::QQuickGridContainer (QQuickItem * parent) : QQuickItem (parent)
  , m_cols            (1)
  , m_rows            (1)
  , m_colSpacing      (0)
  , m_rowSpacing      (0)
  , m_capacity        (-1)
  , m_fillEmpty       (true)
  , m_verticalFlow    (false)
  , m_invertDirection (false)
{ }

int QQuickGridContainer::getCols (void) const {
    return m_cols;
}

int QQuickGridContainer::getRows (void) const {
    return m_rows;
}

int QQuickGridContainer::getColSpacing (void) const {
    return m_colSpacing;
}

int QQuickGridContainer::getRowSpacing (void) const {
    return m_rowSpacing;
}

int QQuickGridContainer::getCapacity (void) const {
    return m_capacity;
}

bool QQuickGridContainer::getFillEmpty (void) const {
    return m_fillEmpty;
}

bool QQuickGridContainer::getVerticalFlow (void) const {
    return m_verticalFlow;
}

bool QQuickGridContainer::getInvertDirection (void) const {
    return m_invertDirection;
}

void QQuickGridContainer::setCols (int cols) {
    if (m_cols != cols) {
        m_cols = cols;
        emit colsChanged (cols);
    }
}

void QQuickGridContainer::setRows (int rows) {
    if (m_rows != rows) {
        m_rows = rows;
        emit rowsChanged (rows);
    }
}

void QQuickGridContainer::setColSpacing (int colSpacing) {
    if (m_colSpacing != colSpacing) {
        m_colSpacing = colSpacing;
        emit colSpacingChanged (colSpacing);
    }
}

void QQuickGridContainer::setRowSpacing (int rowSpacing) {
    if (m_rowSpacing != rowSpacing) {
        m_rowSpacing = rowSpacing;
        emit rowSpacingChanged (rowSpacing);
    }
}

void QQuickGridContainer::setCapacity (int capacity) {
    if (m_capacity != capacity) {
        m_capacity = capacity;
        emit capacityChanged (capacity);
    }
}

void QQuickGridContainer::setFillEmpty (bool fillEmpty) {
    if (m_fillEmpty != fillEmpty) {
        m_fillEmpty = fillEmpty;
        emit fillEmptyChanged (fillEmpty);
    }
}

void QQuickGridContainer::setVerticalFlow (bool verticalFlow) {
    if (m_verticalFlow != verticalFlow) {
        m_verticalFlow = verticalFlow;
        emit verticalFlowChanged (verticalFlow);
    }
}

void QQuickGridContainer::setInvertDirection (bool invertDirection) {
    if (m_invertDirection != invertDirection) {
        m_invertDirection = invertDirection;
        emit invertDirectionChanged (invertDirection);
    }
}

void QQuickGridContainer::classBegin (void) {
    connect (this, &QQuickGridContainer::childrenChanged,        this, &QQuickGridContainer::polish);
    connect (this, &QQuickGridContainer::visibleChanged,         this, &QQuickGridContainer::polish);
    connect (this, &QQuickGridContainer::widthChanged,           this, &QQuickGridContainer::polish);
    connect (this, &QQuickGridContainer::heightChanged,          this, &QQuickGridContainer::polish);
    connect (this, &QQuickGridContainer::colsChanged,            this, &QQuickGridContainer::polish);
    connect (this, &QQuickGridContainer::rowsChanged,            this, &QQuickGridContainer::polish);
    connect (this, &QQuickGridContainer::rowSpacingChanged,      this, &QQuickGridContainer::polish);
    connect (this, &QQuickGridContainer::colSpacingChanged,      this, &QQuickGridContainer::polish);
    connect (this, &QQuickGridContainer::capacityChanged,        this, &QQuickGridContainer::polish);
    connect (this, &QQuickGridContainer::fillEmptyChanged,       this, &QQuickGridContainer::polish);
    connect (this, &QQuickGridContainer::verticalFlowChanged,    this, &QQuickGridContainer::polish);
    connect (this, &QQuickGridContainer::invertDirectionChanged, this, &QQuickGridContainer::polish);
}

void QQuickGridContainer::componentComplete (void) {
    polish ();
}

void QQuickGridContainer::updatePolish (void) {
    const QList<QQuickItem *> childrenList = childItems ();
    /// find max children size and count
    int count = 0;
    qreal maxChildWidth = 0;
    qreal maxChildHeight = 0;
    for (QList<QQuickItem *>::const_iterator it = childrenList.constBegin (); it != childrenList.constEnd (); it++) {
        QQuickItem * child = (* it);
        if (child != Q_NULLPTR && !child->inherits ("QQuickRepeater") && (child->isVisible () || !m_fillEmpty)) {
            if (child->implicitWidth () > maxChildWidth) {
                maxChildWidth = child->implicitWidth ();
            }
            if (child->implicitHeight () > maxChildHeight) {
                maxChildHeight = child->implicitHeight ();
            }
            count++;
        }
    }
    /// recompute rows count
    int tmpRows = 1;
    if (m_cols > 0) {
        if (m_capacity > 0) {
            tmpRows = qCeil (qreal (m_capacity) / qreal (m_cols));
        }
        else if (count > 0) {
            tmpRows = qCeil (qreal (count) / qreal (m_cols));
        }
    }
    setRows (tmpRows);
    /// recompute implicit size
    setImplicitWidth  ((m_cols * maxChildWidth)  + ((m_cols -1) * m_colSpacing));
    setImplicitHeight ((m_rows * maxChildHeight) + ((m_rows -1) * m_rowSpacing));
    /// relayout children
    const qreal layoutWidth  = width ();
    const qreal layoutHeight = height ();
    const qreal itemWidth  = ((qreal (layoutWidth  + m_colSpacing) / qreal (m_cols)) - qreal (m_colSpacing));
    const qreal itemHeight = ((qreal (layoutHeight + m_rowSpacing) / qreal (m_rows)) - qreal (m_rowSpacing));
    const qreal stepX = (itemWidth  + qreal (m_colSpacing));
    const qreal stepY = (itemHeight + qreal (m_rowSpacing));
    qreal curX = (m_invertDirection ? layoutWidth  - itemWidth  : 0);
    qreal curY = (m_invertDirection ? layoutHeight - itemHeight : 0);
    if (m_rows > 0 && m_cols > 0) {
        int nb = 0;
        for (QList<QQuickItem *>::const_iterator it = childrenList.constBegin (); it != childrenList.constEnd (); it++) {
            QQuickItem * child = (* it);
            if (child != Q_NULLPTR && !child->inherits ("QQuickRepeater") && (child->isVisible () || !m_fillEmpty)) {
                child->setX (curX);
                child->setY (curY);
                child->setWidth (itemWidth);
                child->setHeight (itemHeight);
                if (!m_verticalFlow) { // horizontal
                    if ((nb +1) % m_cols > 0) { // next column
                        curX = (m_invertDirection ? curX - stepX : curX + stepX);
                    }
                    else { // next row
                        curX = (m_invertDirection ? layoutWidth - itemWidth : 0);
                        curY = (m_invertDirection ? curY - stepY : curY + stepY);
                    }
                }
                else { // vertical
                    if ((nb +1) % m_rows > 0) { // next row
                        curY = (m_invertDirection ? curY - stepY : curY + stepY);
                    }
                    else { // next column
                        curY = (m_invertDirection ? layoutHeight - itemHeight : 0);
                        curX = (m_invertDirection ? curX - stepX : curX + stepX);
                    }
                }
                nb++;
            }
        }
    }
}

void QQuickGridContainer::itemChange (QQuickItem::ItemChange change, const QQuickItem::ItemChangeData & value) {
    if (change == QQuickItem::ItemChildAddedChange) {
        QQuickItem * child = value.item;
        if (child != Q_NULLPTR) {
            connect (child, &QQuickItem::visibleChanged,        this, &QQuickGridContainer::polish, Qt::UniqueConnection);
            connect (child, &QQuickItem::implicitWidthChanged,  this, &QQuickGridContainer::polish, Qt::UniqueConnection);
            connect (child, &QQuickItem::implicitHeightChanged, this, &QQuickGridContainer::polish, Qt::UniqueConnection);
        }
    }
}
