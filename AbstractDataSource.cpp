#include "AbstractDataSource.h"

#include <QVariant>

AbstractTreeItem::AbstractTreeItem(ItemType type, AbstractTreeItem *parent)
    : parentItem(parent), m_isSelectable(false), m_type(type)
{
    itemData.append(QVariant());
    setParent(parent);
}

AbstractTreeItem::AbstractTreeItem(ItemType type, NuclideId id, const QList<QVariant> &data, bool selectable, AbstractTreeItem *parent)
    : nid(id), itemData(data), parentItem(parent), m_isSelectable(selectable), m_type(type)
{
    setParent(parent);
}

AbstractTreeItem::AbstractTreeItem(const AbstractTreeItem &original)
    : nid(original.nid), itemData(original.itemData), m_isSelectable(original.m_isSelectable), m_type(original.m_type)
{
}

AbstractTreeItem::~AbstractTreeItem()
{
    qDeleteAll(childItems);
}

AbstractTreeItem *AbstractTreeItem::parent() const
{
    return parentItem;
}

AbstractTreeItem *AbstractTreeItem::child(int row)
{
    return childItems.value(row);
}

int AbstractTreeItem::childCount() const
{
    return childItems.count();
}

int AbstractTreeItem::columnCount() const
{
    return itemData.count();
}

bool AbstractTreeItem::hasParent() const
{
    return parentItem;
}

void AbstractTreeItem::setParent(AbstractTreeItem *parent)
{
    parentItem = parent;
    if (parent)
        parent->childItems.append(this);
}

void AbstractTreeItem::setItemData(const QList<QVariant> &data)
{
    itemData = data;
}

void AbstractTreeItem::setSelectable(bool selectable)
{
    m_isSelectable = selectable;
}

bool AbstractTreeItem::isSelectable() const
{
    return m_isSelectable;
}

QVariant AbstractTreeItem::data(int column) const
{
    return itemData.value(column);
}

int AbstractTreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<AbstractTreeItem*>(this));

    return 0;
}

AbstractTreeItem::ItemType AbstractTreeItem::type() const
{
    return m_type;
}

//Decay::CascadeIdentifier AbstractDataSource::cascade(const AbstractTreeItem *) const
//{
//    return Decay::CascadeIdentifier();
//}


