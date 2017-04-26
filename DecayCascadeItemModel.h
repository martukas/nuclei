#pragma once

#include <QAbstractItemModel>
#include <QSharedPointer>

#include "SchemePlayer.h"

class ENSDFDataSource;

class DecayCascadeItemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit DecayCascadeItemModel(ENSDFDataSource *datasource, QObject *parent = 0);
    ~DecayCascadeItemModel();

    virtual void setDataSource(ENSDFDataSource *datasource);

    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex & index ) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual DecayScheme decay(const QModelIndex &index) const;
//    virtual Decay::CascadeIdentifier cascade(const QModelIndex &index) const;
    
signals:
    
public slots:

private:
    ENSDFDataSource *ds;
    
};
