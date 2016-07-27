
#include "QQuickWrapLeftRightContainer.h"

#include <QElapsedTimer>
#include <QDebug>
#include <qmath.h>

QQuickWrapLeftRightContainerBreaker::QQuickWrapLeftRightContainerBreaker (QQuickItem * parent) : QQuickItem (parent) { }

QQuickWrapLeftRightContainer::QQuickWrapLeftRightContainer (QQuickItem * parent) : QQuickItem (parent)
  , m_spacing (0)
  , m_dontWrap (false)
{ }

int QQuickWrapLeftRightContainer::getSpacing (void) const {
    return m_spacing;
}

bool QQuickWrapLeftRightContainer::getDontWrap (void) const {
    return m_dontWrap;
}

void QQuickWrapLeftRightContainer::setSpacing (int spacing) {
    if (m_spacing != spacing) {
        m_spacing = spacing;
        emit spacingChanged (spacing);
    }
}

void QQuickWrapLeftRightContainer::setDontWrap (bool dontWrap) {
    if (m_dontWrap != dontWrap) {
        m_dontWrap = dontWrap;
        emit dontWrapChanged (dontWrap);
    }
}

void QQuickWrapLeftRightContainer::classBegin (void) {
    connect (this, &QQuickWrapLeftRightContainer::childrenChanged, this, &QQuickWrapLeftRightContainer::polish);
    connect (this, &QQuickWrapLeftRightContainer::visibleChanged,  this, &QQuickWrapLeftRightContainer::polish);
    connect (this, &QQuickWrapLeftRightContainer::widthChanged,    this, &QQuickWrapLeftRightContainer::polish);
    connect (this, &QQuickWrapLeftRightContainer::heightChanged,   this, &QQuickWrapLeftRightContainer::polish);
    connect (this, &QQuickWrapLeftRightContainer::spacingChanged,  this, &QQuickWrapLeftRightContainer::polish);
    connect (this, &QQuickWrapLeftRightContainer::dontWrapChanged, this, &QQuickWrapLeftRightContainer::polish);
}

void QQuickWrapLeftRightContainer::componentComplete (void) {
    polish ();
}

static inline bool isWrapper (QQuickItem * item) {
    QQuickWrapLeftRightContainerBreaker * wrapper = qobject_cast<QQuickWrapLeftRightContainerBreaker *> (item);
    return (wrapper != Q_NULLPTR);
}

static inline QQuickItem * keepBiggestImplicitHeight (QQuickItem * item, QQuickItem * ref = Q_NULLPTR) {
    return (ref == Q_NULLPTR ||
            (ref != Q_NULLPTR && ref->implicitHeight () < item->implicitHeight ())
            ? item
            : ref);
}

static inline void centerItemInHeight (QQuickItem * item, // item to center
                                       qreal containerHeight, // height of container
                                       qreal containerOffset = 0) { // offset to the container
    item->setY (containerOffset + (containerHeight - item->height ()) * 0.5);
}

static inline void centerItemInHeightWithBaseline (QQuickItem * item, // item to center
                                                   qreal containerHeight, // height of container
                                                   qreal refHeight, // height of the reference item
                                                   qreal refBaseline, // baseline offset of the reference item
                                                   qreal containerOffset = 0) { // offset to the container
    item->setY (containerOffset + ((containerHeight - refHeight) * 0.5) + refBaseline - item->baselineOffset ());
}

void QQuickWrapLeftRightContainer::updatePolish (void) {
    const QList<QQuickItem *> childrenList = childItems ();
    /// find left and right items and compare heights
    bool wrapperFound = false;
    qreal tmpWidth = 0;
    QQuickItem * biggestHeightLeftItem = Q_NULLPTR;
    QQuickItem * biggestHeightRightItem = Q_NULLPTR;
    QQuickItem * biggestHeightLeftTextItem = Q_NULLPTR;
    QQuickItem * biggestHeightRightTextItem = Q_NULLPTR;
    QVector<QQuickItem *> leftItems;
    QVector<QQuickItem *> rightItems;
    leftItems.reserve  (childrenList.count ());
    rightItems.reserve (childrenList.count ());
    for (QList<QQuickItem *>::const_iterator it = childrenList.constBegin (); it != childrenList.constEnd (); it++) {
        QQuickItem * child = (* it);
        if (child != Q_NULLPTR && !child->inherits ("QQuickRepeater") && child->isVisible ()) {
            if (isWrapper (child)) {
                wrapperFound = true;
            }
            else {
                if (tmpWidth > 0) {
                    tmpWidth += m_spacing;
                }
                tmpWidth += child->implicitWidth ();
                if (wrapperFound) {
                    rightItems.prepend (child);
                    biggestHeightRightItem = keepBiggestImplicitHeight (child, biggestHeightRightItem);
                    if (child->baselineOffset () != 0.0) {
                        biggestHeightRightTextItem = keepBiggestImplicitHeight (child, biggestHeightRightTextItem);
                    }
                }
                else {
                    leftItems.append (child);
                    biggestHeightLeftItem = keepBiggestImplicitHeight (child, biggestHeightLeftItem);
                    if (child->baselineOffset () != 0.0) {
                        biggestHeightLeftTextItem = keepBiggestImplicitHeight (child, biggestHeightLeftTextItem);
                    }
                }
            }
        }
    }
    /// resize layout
    setImplicitWidth (tmpWidth);
    const bool  mustWrap    = (!m_dontWrap ? width () < implicitWidth () : false);
    const qreal layoutWidth = width ();
    const qreal leftHeight  = (biggestHeightLeftItem  ? biggestHeightLeftItem->implicitHeight ()  : 0);
    const qreal rightHeight = (biggestHeightRightItem ? biggestHeightRightItem->implicitHeight () : 0);
    setImplicitHeight (mustWrap ? (leftHeight + m_spacing + rightHeight) : qMax (leftHeight, rightHeight));
    const qreal layoutHeight = height ();
    /// position left items
    qreal curX = 0;
    for (QVector<QQuickItem *>::const_iterator it = leftItems.constBegin (); it != leftItems.constEnd (); it++) {
        QQuickItem * child = (* it);
        child->setWidth  (child->implicitWidth ());
        child->setHeight (child->implicitHeight ());
        child->setX (curX);
        curX += child->width ();
        curX += m_spacing;
        if (child->baselineOffset () != 0.0 && biggestHeightLeftTextItem != Q_NULLPTR && child != biggestHeightLeftTextItem) {
            centerItemInHeightWithBaseline (child,
                                            (mustWrap ? leftHeight : layoutHeight),
                                            biggestHeightLeftTextItem->implicitHeight (),
                                            biggestHeightLeftTextItem->baselineOffset ());
        }
        else {
            centerItemInHeight (child, (mustWrap ? leftHeight : layoutHeight));
        }
    }
    /// position right items
    curX = layoutWidth;
    const qreal offset = (mustWrap ? leftHeight + m_spacing : 0);
    for (QVector<QQuickItem *>::const_iterator it = rightItems.constBegin (); it != rightItems.constEnd (); it++) {
        QQuickItem * child = (* it);
        child->setWidth  (child->implicitWidth ());
        child->setHeight (child->implicitHeight ());
        curX -= child->width ();
        child->setX (curX);
        curX -= m_spacing;
        if (child->baselineOffset () != 0.0 && biggestHeightRightTextItem != Q_NULLPTR && child != biggestHeightRightTextItem) {
            centerItemInHeightWithBaseline (child,
                                            (mustWrap ? rightHeight : layoutHeight),
                                            biggestHeightRightTextItem->implicitHeight (),
                                            biggestHeightRightTextItem->baselineOffset (),
                                            offset);
        }
        else {
            centerItemInHeight (child, (mustWrap ? rightHeight : layoutHeight), offset);
        }
    }
}

void QQuickWrapLeftRightContainer::itemChange (QQuickItem::ItemChange change, const QQuickItem::ItemChangeData & value) {
    if (change == QQuickItem::ItemChildAddedChange) {
        QQuickItem * child = value.item;
        if (child != Q_NULLPTR) {
            connect (child, &QQuickItem::visibleChanged,
                     this,  &QQuickWrapLeftRightContainer::polish, Qt::UniqueConnection);
            connect (child, &QQuickItem::implicitWidthChanged,
                     this,  &QQuickWrapLeftRightContainer::polish, Qt::UniqueConnection);
            connect (child, &QQuickItem::implicitHeightChanged,
                     this,  &QQuickWrapLeftRightContainer::polish, Qt::UniqueConnection);
        }
    }
}
