#ifndef ABSTRACTDATASOURCE_H
#define ABSTRACTDATASOURCE_H

#include <QObject>
#include <QStringList>
#include <QSharedPointer>

class Decay;

class AbstractTreeItem
{
public:
    enum ItemType {
        UnknownType,
        DaughterType,
        DecayType,
        CascadeType
    };

    explicit AbstractTreeItem(AbstractTreeItem *parent = 0);
    explicit AbstractTreeItem(ItemType type, unsigned int A, const QList<QVariant> &data, bool selectable, AbstractTreeItem *parent = 0);
    virtual ~AbstractTreeItem();

    virtual AbstractTreeItem *parent() const;
    virtual AbstractTreeItem *child(int row);

    virtual int childCount() const;
    virtual int columnCount() const;
    virtual bool hasParent() const;

    virtual bool isSelectable() const;

    virtual QVariant data(int column) const;

    virtual int row() const;

    virtual unsigned int A() const;
    virtual ItemType type() const;

protected:
    unsigned int m_A;
    QList<AbstractTreeItem*> childItems;
    QList<QVariant> itemData;
    AbstractTreeItem *parentItem;
    bool m_isSelectable;
    ItemType m_type;
};


class AbstractDataSource : public QObject
{
    Q_OBJECT

public:
    explicit AbstractDataSource(QObject *parent = 0) : QObject(parent) {}
    virtual ~AbstractDataSource() {}

    virtual AbstractTreeItem * rootItem() const = 0;

    virtual QSharedPointer<Decay> decay(const AbstractTreeItem *item) const = 0;
};

#endif // DATASOURCE_H
