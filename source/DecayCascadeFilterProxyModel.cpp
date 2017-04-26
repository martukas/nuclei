#include "DecayCascadeFilterProxyModel.h"

DecayCascadeFilterProxyModel::DecayCascadeFilterProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

bool DecayCascadeFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent))
        return true;

    // accept if parent matches
    QModelIndex parent = source_parent;
    while (parent.isValid()) {
        if (filterAcceptsThisRow(parent.row(), parent.parent()))
            return true;
        parent = parent.parent();
    }

    // accept if child matches (only direct children. not recursive!!)
    QModelIndex item = sourceModel()->index(source_row, 0, source_parent);
    if (!item.isValid())
        return false;
    //check if there are children
    int childCount = item.model()->rowCount(item);
    for (int i = 0; i < childCount; i++)
        if (filterAcceptsThisRow(i, item))
            return true;

    return false;
}

bool DecayCascadeFilterProxyModel::filterAcceptsThisRow(int source_row, const QModelIndex &source_parent) const
{
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}
