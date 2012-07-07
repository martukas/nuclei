#include "AbstractDataSource.h"

#include <QVariant>

AbstractTreeItem::AbstractTreeItem(AbstractTreeItem *parent)
    : m_A(0), parentItem(parent), m_isSelectable(false), m_type(UnknownType)
{
    itemData.append(QVariant());
    if (parent)
        parent->childItems.append(this);
}

AbstractTreeItem::AbstractTreeItem(ItemType type, unsigned int A, const QList<QVariant> &data, bool selectable, AbstractTreeItem *parent)
    : m_A(A), itemData(data), parentItem(parent), m_isSelectable(selectable), m_type(type)
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

unsigned int AbstractTreeItem::A() const
{
    return m_A;
}

AbstractTreeItem::ItemType AbstractTreeItem::type() const
{
    return m_type;
}



