#ifndef ABSTRACTDATASOURCE_H
#define ABSTRACTDATASOURCE_H

#include <QObject>
#include <QStringList>
#include <QSharedPointer>

#include "Decay.h"

class AbstractTreeItem
{
public:
    enum ItemType {
        UnknownType,
        RootType,
        DaughterType,
        DecayType,
        CascadeType
    };

    explicit AbstractTreeItem(ItemType type = UnknownType, AbstractTreeItem *parent = 0);
    explicit AbstractTreeItem(ItemType type, unsigned int A, unsigned int Z, const QList<QVariant> &data, bool selectable, AbstractTreeItem *parent = 0);
    /**
     * @brief This copy constructor creates a standalone copy without links to parent or children
     * @param original
     */
    explicit AbstractTreeItem(const AbstractTreeItem &original);
    virtual ~AbstractTreeItem();

    virtual AbstractTreeItem *parent() const;
    virtual AbstractTreeItem *child(int row);

    virtual int childCount() const;
    virtual int columnCount() const;
    virtual bool hasParent() const;

    virtual void setParent(AbstractTreeItem *parent);
    virtual void setItemData(const QList<QVariant> &data);

    virtual void setSelectable(bool selectable);
    virtual bool isSelectable() const;

    virtual QVariant data(int column) const;

    virtual int row() const;

    virtual unsigned int A() const;
    virtual unsigned int Z() const;
    virtual ItemType type() const;

protected:
    unsigned int m_A, m_Z;
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
    virtual Decay::CascadeIdentifier cascade(const AbstractTreeItem *item) const;
};

#endif // DATASOURCE_H
