#pragma once

#include <QAbstractItemModel>
#include <QSharedPointer>

#include "ENSDFDataSource.h"
#include <QPointer>

class DecayCascadeItemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit DecayCascadeItemModel(QPointer<ENSDFDataSource> datasource,
                                   QObject *parent = 0);
    ~DecayCascadeItemModel();

    virtual void setDataSource(ENSDFDataSource *datasource);

    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex & index ) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual DecayScheme decay(const QModelIndex &index, bool merge) const;
//    virtual Decay::CascadeIdentifier cascade(const QModelIndex &index) const;
    
signals:
    
public slots:

private:
    QPointer<ENSDFDataSource> ds;
    
};
