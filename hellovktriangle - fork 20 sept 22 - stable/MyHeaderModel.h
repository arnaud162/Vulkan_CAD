#ifndef MYHEADERMODEL_H
#define MYHEADERMODEL_H

#include <QtGui>
#include <QHeaderView>
#include <QTableWidget>

#endif // MYHEADERMODEL_H
class MyHeaderModel : public QAbstractItemModel
{
public:
    MyHeaderModel(QObject *parent = 0) : QAbstractItemModel(parent) {}
    int columnCount(const QModelIndex &parent = QModelIndex()) const { return 2; }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const { return QVariant(); }
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const { return QModelIndex(); }
    QModelIndex parent(const QModelIndex &index) const { return QModelIndex(); }
    int rowCount(const QModelIndex &parent = QModelIndex()) const { return 0; }
};

class MyHeader : public QHeaderView
{
    Q_OBJECT
public:
    MyHeader(QHeaderView *header, QWidget *parent = 0) : QHeaderView(Qt::Horizontal, header), mainHeader(header)
    {
        setModel(new MyHeaderModel(this));
        // This example uses hardcoded groups, you can extend
        // this yourself to save the groups
        // Group 1 is 0-2 and Group 2 is 3-4
        resizeSection(0, getSectionSizes(0, 2));
        resizeSection(1, getSectionSizes(3, 4));
        connect(this, SIGNAL(sectionResized(int,int,int)), this, SLOT(updateSizes()));
        connect(((QTableWidget *)(mainHeader->parentWidget()))->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateOffset()));
        setGeometry(0, 0, header->width(), header->height());
        updateOffset();
        mainHeader->installEventFilter(this);
    }
public slots:
    void updateSizes()
    {
        setOffset(mainHeader->offset());
        mainHeader->resizeSection(2, mainHeader->sectionSize(2) + (sectionSize(0) - getSectionSizes(0, 2)));
        mainHeader->resizeSection(4, mainHeader->sectionSize(4) + (sectionSize(1) - getSectionSizes(3, 4)));
    }
    void updateOffset()
    {
        setOffset(mainHeader->offset());
    }
protected:
    bool eventFilter(QObject *o, QEvent *e)
    {
        if (o == mainHeader) {
            if (e->type() == QEvent::Resize) {
                setOffset(mainHeader->offset());
                setGeometry(0, 0, mainHeader->width(), mainHeader->height());
            }
            return false;
        }
        return QHeaderView::eventFilter(o, e);
    }
private:
    int getSectionSizes(int first, int second)
    {
        int size = 0;
        for (int a=first;a<=second;++a)
            size += mainHeader->sectionSize(a);
        return size;
    }
    QHeaderView *mainHeader;

};
