#ifndef ABSTRACTDATASOURCE_H
#define ABSTRACTDATASOURCE_H

#include <QObject>
#include <QStringList>
#include <QSharedPointer>

class Decay;

class AbstractTreeItem
{
public:
    explicit AbstractTreeItem(AbstractTreeItem *parent = 0);
    explicit AbstractTreeItem(const QList<QVariant> &data, bool selectable, AbstractTreeItem *parent = 0);
    virtual ~AbstractTreeItem();

    virtual AbstractTreeItem *parent() const;
    virtual AbstractTreeItem *child(int row);

    virtual int childCount() const;
    virtual int columnCount() const;
    virtual bool hasParent() const;

    virtual bool isSelectable() const;

    virtual QVariant data(int column) const;

    virtual int row() const;

protected:
    QList<AbstractTreeItem*> childItems;
    QList<QVariant> itemData;
    AbstractTreeItem *parentItem;
    bool m_isSelectable;
};


class AbstractDataSource : public QObject
{
    Q_OBJECT

public:
    explicit AbstractDataSource(QObject *parent = 0) : QObject(parent) {};
    virtual ~AbstractDataSource() {};

    virtual AbstractTreeItem * rootItem() const = 0;

    virtual QSharedPointer<Decay> decay(const AbstractTreeItem *item) const = 0;
};

#endif // DATASOURCE_H
