#include "TreeView.h"
#if !defined(Q_OS_MAC)
#include <QxtGui/QxtItemDelegate>
#endif

TreeView::TreeView(QWidget *parent) :
    QTreeView(parent)
{
    connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(processClick(QModelIndex)));
#if !defined(Q_OS_MAC)
    QxtItemDelegate *delegate = new QxtItemDelegate(this);
    delegate->setDecorationStyle(Qxt::Buttonlike);
    setRootIsDecorated(false);
    setItemDelegate(delegate);
#endif
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
