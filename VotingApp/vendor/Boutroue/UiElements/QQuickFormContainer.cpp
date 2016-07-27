
#include "QQuickFormContainer.h"

#include <QtMath>

QQuickFormContainer::QQuickFormContainer (QQuickItem * parent) : QQuickItem (parent)
  , m_colSpacing (0)
  , m_rowSpacing (0)
{ }

int QQuickFormContainer::getColSpacing (void) const {
    return m_colSpacing;
}

int QQuickFormContainer::getRowSpacing (void) const {
    return m_rowSpacing;
}

void QQuickFormContainer::setColSpacing (int colSpacing) {
    if (m_colSpacing != colSpacing) {
        m_colSpacing = colSpacing;
        emit colSpacingChanged (colSpacing);
    }
}

void QQuickFormContainer::setRowSpacing (int rowSpacing) {
    if (m_rowSpacing != rowSpacing) {
        m_rowSpacing = rowSpacing;
        emit rowSpacingChanged (rowSpacing);
    }
}

void QQuickFormContainer::classBegin (void) {
    connect (this, &QQuickFormContainer::childrenChanged,   this, &QQuickFormContainer::polish);
    connect (this, &QQuickFormContainer::visibleChanged,    this, &QQuickFormContainer::polish);
    connect (this, &QQuickFormContainer::widthChanged,      this, &QQuickFormContainer::polish);
    connect (this, &QQuickFormContainer::heightChanged,     this, &QQuickFormContainer::polish);
    connect (this, &QQuickFormContainer::rowSpacingChanged, this, &QQuickFormContainer::polish);
    connect (this, &QQuickFormContainer::colSpacingChanged, this, &QQuickFormContainer::polish);
}

void QQuickFormContainer::componentComplete (void) {
    polish ();
}

void QQuickFormContainer::updatePolish (void) {
    const QList<QQuickItem *> childrenList = childItems ();
    /// find max children size and count
    int count = 0;
    qreal leftMaxChildWidth  = 0;
    qreal rightMaxChildWidth = 0;
    QList<qreal> leftChildHeight;
    QList<qreal> rightChildHeight;
    for (QList<QQuickItem *>::const_iterator it = childrenList.constBegin (); it != childrenList.constEnd (); it++) {
        QQuickItem * child = (* it);
        if (child != Q_NULLPTR && !child->inherits ("QQuickRepeater") && child->isVisible ()) {
            if (count % 2 == 0) {
                if (child->implicitWidth () > leftMaxChildWidth) {
                    leftMaxChildWidth = child->implicitWidth ();
                }
                leftChildHeight.append (child->implicitHeight ());
            }
            else {
                if (child->implicitWidth () > rightMaxChildWidth) {
                    rightMaxChildWidth = child->implicitWidth ();
                }
                rightChildHeight.append (child->implicitHeight ());
            }
            count++;
        }
    }
    QList<qreal> childHeight;
    int line = 0;
    qreal total = 0;
    while (line < leftChildHeight.size () && line < rightChildHeight.size ()) {
        const qreal tmp = qMax (leftChildHeight.at (line), rightChildHeight.at (line));
        childHeight.append (tmp);
        total += tmp;
        line++;
    }
    /// recompute rows count
    const int cols = 2;
    const int rows = qCeil (qreal (count) / qreal (cols));
    /// recompute implicit size
    setImplicitWidth  (leftMaxChildWidth + rightMaxChildWidth + m_colSpacing);
    setImplicitHeight (total + (rows > 1 ? m_rowSpacing * (rows -1) : 0));
    /// relayout children
    const qreal layoutWidth    = width ();
    const qreal leftItemWidth  = (leftMaxChildWidth);
    const qreal rightItemWidth = (layoutWidth - leftItemWidth - m_colSpacing);
    if (rows > 0 && cols > 0) {
        int nb = 0;
        int line = 0;
        qreal currentY = 0;
        for (QList<QQuickItem *>::const_iterator it = childrenList.constBegin (); it != childrenList.constEnd (); it++) {
            QQuickItem * child = (* it);
            if (child != Q_NULLPTR && !child->inherits ("QQuickRepeater") && child->isVisible ()) {
                const qreal lineHeight = childHeight.at (line);
                child->setY (currentY);
                child->setHeight (lineHeight);
                if (nb % 2 == 0) {
                    child->setX (0);
                    child->setWidth (leftItemWidth);
                }
                else {
                    child->setX (leftItemWidth + m_colSpacing);
                    child->setWidth (rightItemWidth);
                    currentY += (lineHeight + m_rowSpacing);
                    line++;
                }
                nb++;
            }
        }
    }
}

void QQuickFormContainer::itemChange (QQuickItem::ItemChange change, const QQuickItem::ItemChangeData & value) {
    if (change == QQuickItem::ItemChildAddedChange) {
        QQuickItem * child = value.item;
        if (child != Q_NULLPTR) {
            connect (child, &QQuickItem::visibleChanged,        this, &QQuickFormContainer::polish, Qt::UniqueConnection);
            connect (child, &QQuickItem::implicitWidthChanged,  this, &QQuickFormContainer::polish, Qt::UniqueConnection);
            connect (child, &QQuickItem::implicitHeightChanged, this, &QQuickFormContainer::polish, Qt::UniqueConnection);
        }
    }
}
