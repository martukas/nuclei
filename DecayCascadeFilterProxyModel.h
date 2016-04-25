#ifndef DECAYCASCADEFILTERPROXYMODEL_H
#define DECAYCASCADEFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class DecayCascadeFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit DecayCascadeFilterProxyModel(QObject *parent = 0);
    
signals:
    
public slots:

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const;
    virtual bool filterAcceptsThisRow(int source_row, const QModelIndex &source_parent) const;

};

#endif // DECAYCASCADEFILTERPROXYMODEL_H
