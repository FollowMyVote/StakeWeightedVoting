#ifndef QQUICKWRAPLEFTRIGHTCONTAINER_H
#define QQUICKWRAPLEFTRIGHTCONTAINER_H

#include <QObject>
#include <QQuickItem>

class QQuickWrapLeftRightContainerBreaker : public QQuickItem {
    Q_OBJECT

public:
    explicit QQuickWrapLeftRightContainerBreaker (QQuickItem * parent = Q_NULLPTR);
};

class QQuickWrapLeftRightContainer : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY (int  spacing  READ getSpacing  WRITE setSpacing  NOTIFY spacingChanged)
    Q_PROPERTY (bool dontWrap READ getDontWrap WRITE setDontWrap NOTIFY dontWrapChanged)

public:
    explicit QQuickWrapLeftRightContainer (QQuickItem * parent = Q_NULLPTR);

    int  getSpacing  (void) const;
    bool getDontWrap (void) const;

public slots:
    void setSpacing  (int  spacing);
    void setDontWrap (bool dontWrap);

signals:
    void spacingChanged  (int  spacing);
    void dontWrapChanged (bool dontWrap);

protected:
    void classBegin        (void);
    void componentComplete (void);
    void updatePolish      (void);
    void itemChange        (ItemChange change, const ItemChangeData & value);

private:
    int  m_spacing;
    bool m_dontWrap;
};

#endif // QQUICKWRAPLEFTRIGHTCONTAINER_H
