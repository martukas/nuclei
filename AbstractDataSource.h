#ifndef ABSTRACTDATASOURCE_H
#define ABSTRACTDATASOURCE_H

#include <QObject>
#include <QStringList>
#include <QSharedPointer>

#include "XDecay.h"

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
    explicit AbstractTreeItem(ItemType type, NuclideId id, const QList<QVariant> &data, bool selectable, AbstractTreeItem *parent = 0);
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

    virtual NuclideId id() const { return nid; }
    virtual ItemType type() const;

protected:
    NuclideId nid;
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

    virtual XDecayPtr decay(const AbstractTreeItem *item) const = 0;
//    virtual XDecay::CascadeIdentifier cascade(const AbstractTreeItem *item) const;
};

#endif // DATASOURCE_H
