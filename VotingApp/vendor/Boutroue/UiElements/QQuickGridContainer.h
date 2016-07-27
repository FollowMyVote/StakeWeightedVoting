#ifndef QQUICKGRIDCONTAINER_H
#define QQUICKGRIDCONTAINER_H

#include <QObject>
#include <QQuickItem>
#include <QQmlParserStatus>
#include <QQmlListProperty>
#include <QTimer>

class QQuickGridContainer : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY (int  cols            READ getCols            WRITE setCols            NOTIFY colsChanged)
    Q_PROPERTY (int  rows            READ getRows            WRITE setRows            NOTIFY rowsChanged)
    Q_PROPERTY (int  colSpacing      READ getColSpacing      WRITE setColSpacing      NOTIFY colSpacingChanged)
    Q_PROPERTY (int  rowSpacing      READ getRowSpacing      WRITE setRowSpacing      NOTIFY rowSpacingChanged)
    Q_PROPERTY (int  capacity        READ getCapacity        WRITE setCapacity        NOTIFY capacityChanged)
    Q_PROPERTY (bool fillEmpty       READ getFillEmpty       WRITE setFillEmpty       NOTIFY fillEmptyChanged)
    Q_PROPERTY (bool verticalFlow    READ getVerticalFlow    WRITE setVerticalFlow    NOTIFY verticalFlowChanged)
    Q_PROPERTY (bool invertDirection READ getInvertDirection WRITE setInvertDirection NOTIFY invertDirectionChanged)

public:
    explicit QQuickGridContainer (QQuickItem * parent = Q_NULLPTR);

    int  getCols            (void) const;
    int  getRows            (void) const;
    int  getColSpacing      (void) const;
    int  getRowSpacing      (void) const;
    int  getCapacity        (void) const;
    bool getFillEmpty       (void) const;
    bool getVerticalFlow    (void) const;
    bool getInvertDirection (void) const;

public slots:
    void setCols            (int cols);
    void setRows            (int rows);
    void setColSpacing      (int colSpacing);
    void setRowSpacing      (int rowSpacing);
    void setCapacity        (int capacity);
    void setFillEmpty       (bool fillEmpty);
    void setVerticalFlow    (bool verticalFlow);
    void setInvertDirection (bool invertDirection);

signals:
    void colsChanged            (int cols);
    void rowsChanged            (int rows);
    void colSpacingChanged      (int colSpacing);
    void rowSpacingChanged      (int rowSpacing);
    void capacityChanged        (int capacity);
    void fillEmptyChanged       (bool fillEmpty);
    void verticalFlowChanged    (bool verticalFlow);
    void invertDirectionChanged (bool invertDirection);

protected:
    void classBegin        (void);
    void componentComplete (void);
    void updatePolish      (void);
    void itemChange        (ItemChange change, const ItemChangeData & value);

private:
    int m_cols;
    int m_rows;
    int m_colSpacing;
    int m_rowSpacing;
    int m_capacity;
    bool m_fillEmpty;
    bool m_verticalFlow;
    bool m_invertDirection;
};

#endif // QQUICKGRIDCONTAINER_H
