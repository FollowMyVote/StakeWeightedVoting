
#include "QQuickStretchColumnContainer.h"

#include <QElapsedTimer>
#include <QDebug>
#include <qmath.h>

QQuickStretchColumnContainer::QQuickStretchColumnContainer (QQuickItem * parent) : QQuickItem (parent)
  , m_spacing (0)
{ }

int QQuickStretchColumnContainer::getSpacing (void) const {
    return m_spacing;
}

void QQuickStretchColumnContainer::setSpacing (int spacing) {
    if (m_spacing != spacing) {
        m_spacing = spacing;
        emit spacingChanged (spacing);
    }
}

void QQuickStretchColumnContainer::classBegin (void) {
    connect (this, &QQuickStretchColumnContainer::childrenChanged, this, &QQuickStretchColumnContainer::polish);
    connect (this, &QQuickStretchColumnContainer::visibleChanged,  this, &QQuickStretchColumnContainer::polish);
    connect (this, &QQuickStretchColumnContainer::widthChanged,    this, &QQuickStretchColumnContainer::polish);
    connect (this, &QQuickStretchColumnContainer::heightChanged,   this, &QQuickStretchColumnContainer::polish);
    connect (this, &QQuickStretchColumnContainer::spacingChanged,  this, &QQuickStretchColumnContainer::polish);
}

void QQuickStretchColumnContainer::componentComplete (void) {
    polish ();
}

void QQuickStretchColumnContainer::updatePolish (void) {
    const QList<QQuickItem *> childrenList = childItems ();
    /// find items and stretchers
    qreal tmpW = 0;
    qreal tmpH = 0;
    int nbItems   = 0;
    int nbStretch = 0;
    for (QList<QQuickItem *>::const_iterator it = childrenList.constBegin (); it != childrenList.constEnd (); it++) {
        QQuickItem * child = (* it);
        if (child != Q_NULLPTR && !child->inherits ("QQuickRepeater") && child->isVisible ()) {
            if (child->implicitWidth () > tmpW) {
                tmpW = child->implicitWidth ();
            }
            if (child->implicitHeight () >= 0) {
                tmpH += child->implicitHeight ();
            }
            else {
                nbStretch++;
            }
            nbItems++;
        }
    }
    /// resize layout
    if (nbItems > 1) {
        tmpH += (m_spacing * (nbItems -1));
    }
    setImplicitWidth  (tmpW);
    setImplicitHeight (tmpH);
    const qreal layoutWidth  = width  ();
    const qreal layoutHeight = height ();
    const qreal autoSize = (nbStretch > 0 ? (layoutHeight - tmpH) / qreal (nbStretch) : 0);
    /// position children
    int currY = 0;
    for (QList<QQuickItem *>::const_iterator it = childrenList.constBegin (); it != childrenList.constEnd (); it++) {
        QQuickItem * child = (* it);
        if (child != Q_NULLPTR && !child->inherits ("QQuickRepeater") && child->isVisible ()) {
            if (currY) {
                currY += m_spacing;
            }
            child->setY (currY);
            child->setWidth (layoutWidth);
            child->setHeight (child->implicitHeight () >= 0 ? child->implicitHeight () : autoSize);
            currY += child->height ();
        }
    }
}

void QQuickStretchColumnContainer::itemChange (QQuickItem::ItemChange change, const QQuickItem::ItemChangeData & value) {
    if (change == QQuickItem::ItemChildAddedChange) {
        QQuickItem * child = value.item;
        if (child != Q_NULLPTR) {
            connect (child, &QQuickItem::visibleChanged,
                     this,  &QQuickStretchColumnContainer::polish, Qt::UniqueConnection);
            connect (child, &QQuickItem::implicitWidthChanged,
                     this,  &QQuickStretchColumnContainer::polish, Qt::UniqueConnection);
            connect (child, &QQuickItem::implicitHeightChanged,
                     this,  &QQuickStretchColumnContainer::polish, Qt::UniqueConnection);
        }
    }
}
