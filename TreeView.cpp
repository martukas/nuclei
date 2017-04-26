#include "TreeView.h"
#include <QAbstractItemDelegate>

TreeView::TreeView(QWidget *parent)
  : QTreeView(parent)
{
  connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(processClick(QModelIndex)));
}

void TreeView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
  if (model()->flags(current) & Qt::ItemIsSelectable)
    emit showItem(current);
  QTreeView::currentChanged(current, previous);
}

void TreeView::processClick(const QModelIndex &item)
{
  //if (!(model()->flags(item) & Qt::ItemIsSelectable))
  setExpanded(item, !isExpanded(item));
}
