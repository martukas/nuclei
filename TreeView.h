#pragma once

#include <QTreeView>

class TreeView : public QTreeView
{
  Q_OBJECT
public:
  explicit TreeView(QWidget *parent = 0);

signals:
  void showItem(const QModelIndex &item);

protected:
  void currentChanged(const QModelIndex &current, const QModelIndex &previous);

private slots:
  void processClick(const QModelIndex &item);

};
