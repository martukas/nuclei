#include "AbstractDataSource.h"

#include <QVariant>

AbstractTreeItem::AbstractTreeItem(AbstractTreeItem *parent)
    : parentItem(parent), m_isSelectable(false)
{
    itemData.append(QVariant());
    if (parent)
        parent->childItems.append(this);
}

AbstractTreeItem::AbstractTreeItem(const QList<QVariant> &data, bool selectable, AbstractTreeItem *parent)
    : itemData(data), parentItem(parent), m_isSelectable(selectable)
{
    if (parent)
        parent->childItems.append(this);
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



