#include "DecayCascadeItemModel.h"

#include <QModelIndex>
#include <QString>

#include "ENSDFDataSource.h"

DecayCascadeItemModel::DecayCascadeItemModel(QPointer<ENSDFDataSource> datasource, QObject *parent)
  : QAbstractItemModel(parent), ds(datasource)
{
}

DecayCascadeItemModel::~DecayCascadeItemModel()
{
}

void DecayCascadeItemModel::setDataSource(ENSDFDataSource *datasource)
{
  beginResetModel();
  ds = datasource;
  endResetModel();
}

int DecayCascadeItemModel::columnCount(const QModelIndex &parent) const
{
  if (!ds)
    return 0;

  if (parent.isValid())
    return static_cast<ENSDFTreeItem*>(parent.internalPointer())->columnCount();
  else
    return ds->rootItem()->columnCount();
}

QVariant DecayCascadeItemModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  ENSDFTreeItem *item = static_cast<ENSDFTreeItem*>(index.internalPointer());

  if (role == Qt::DisplayRole)
    return item->data(index.column());

  if (role == Qt::ToolTipRole) {
    if (index.parent().isValid())
      return item->data(index.column()).toString();
    else
      return "Daughter Nuclide: " + item->data(index.column()).toString();
  }

  return QVariant();
}

QModelIndex DecayCascadeItemModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  if (!ds)
    return QModelIndex();

  ENSDFTreeItem *parentItem;

  if (!parent.isValid())
    parentItem = ds->rootItem();
  else
    parentItem = static_cast<ENSDFTreeItem*>(parent.internalPointer());

  ENSDFTreeItem *childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}


int DecayCascadeItemModel::rowCount(const QModelIndex &parent) const
{
  if (!ds)
    return 0;

  ENSDFTreeItem *parentItem;
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    parentItem = ds->rootItem();
  else
    parentItem = static_cast<ENSDFTreeItem*>(parent.internalPointer());

  return parentItem->childCount();
}

Qt::ItemFlags DecayCascadeItemModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return 0;

  Qt::ItemFlags result = Qt::ItemIsEnabled;

  ENSDFTreeItem *it = static_cast<ENSDFTreeItem*>(index.internalPointer());
  if (it && it->isSelectable())
    result |= Qt::ItemIsSelectable;


  return result;
}

DecayScheme DecayCascadeItemModel::decay(const QModelIndex &index,
                                         bool merge) const
{
  if (!index.isValid())
    return DecayScheme();

  ENSDFTreeItem *item = static_cast<ENSDFTreeItem*>(index.internalPointer());

  return ds->decay(item, merge);
}

//Decay::CascadeIdentifier DecayCascadeItemModel::cascade(const QModelIndex &index) const
//{
//    if (!index.isValid())
//        return Decay::CascadeIdentifier();

//    ENSDFTreeItem *item = static_cast<ENSDFTreeItem*>(index.internalPointer());

//    return ds->cascade(item);
//}


QModelIndex DecayCascadeItemModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  ENSDFTreeItem *childItem = static_cast<ENSDFTreeItem*>(index.internalPointer());
  ENSDFTreeItem *parentItem = childItem->parent();

  if (parentItem == ds->rootItem())
    return QModelIndex();

  return createIndex(parentItem->row(), 0, parentItem);
}


