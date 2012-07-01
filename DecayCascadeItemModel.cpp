#include "DecayCascadeItemModel.h"

#include <QModelIndex>
#include <QString>

#include "AbstractDataSource.h"

DecayCascadeItemModel::DecayCascadeItemModel(AbstractDataSource *datasource, QObject *parent)
    : QAbstractItemModel(parent), ds(datasource)
{
}

DecayCascadeItemModel::~DecayCascadeItemModel()
{
}

int DecayCascadeItemModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<AbstractTreeItem*>(parent.internalPointer())->columnCount();
    else
        return ds->rootItem()->columnCount();
}

QVariant DecayCascadeItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
        return QVariant();

    AbstractTreeItem *item = static_cast<AbstractTreeItem*>(index.internalPointer());

    return item->data(index.column());
}

QModelIndex DecayCascadeItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    AbstractTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = ds->rootItem();
    else
        parentItem = static_cast<AbstractTreeItem*>(parent.internalPointer());

    AbstractTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}


int DecayCascadeItemModel::rowCount(const QModelIndex &parent) const
{
    AbstractTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = ds->rootItem();
    else
        parentItem = static_cast<AbstractTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

Qt::ItemFlags DecayCascadeItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags result = Qt::ItemIsEnabled;

    AbstractTreeItem *it = static_cast<AbstractTreeItem*>(index.internalPointer());
    if (it && it->isSelectable())
        result |= Qt::ItemIsSelectable;


    return result;
}


QModelIndex DecayCascadeItemModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    AbstractTreeItem *childItem = static_cast<AbstractTreeItem*>(index.internalPointer());
    AbstractTreeItem *parentItem = childItem->parent();

    if (parentItem == ds->rootItem())
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}


