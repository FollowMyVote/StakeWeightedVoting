#ifndef QQUICKFORMCONTAINER_H
#define QQUICKFORMCONTAINER_H

#include <QObject>
#include <QQuickItem>
#include <QQmlParserStatus>

class QQuickFormContainer : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY (int colSpacing READ getColSpacing WRITE setColSpacing NOTIFY colSpacingChanged)
    Q_PROPERTY (int rowSpacing READ getRowSpacing WRITE setRowSpacing NOTIFY rowSpacingChanged)

public:
    explicit QQuickFormContainer (QQuickItem * parent = Q_NULLPTR);

    int getColSpacing (void) const;
    int getRowSpacing (void) const;

public slots:
    void setColSpacing (int colSpacing);
    void setRowSpacing (int rowSpacing);

signals:
    void colSpacingChanged (int colSpacing);
    void rowSpacingChanged (int rowSpacing);

protected:
    void classBegin        (void);
    void componentComplete (void);
    void updatePolish      (void);
    void itemChange        (ItemChange change, const ItemChangeData & value);

private:
    int m_colSpacing;
    int m_rowSpacing;
};

#endif // QQUICKFORMCONTAINER_H
