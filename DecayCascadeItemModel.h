#ifndef DECAYCASCADEITEMMODEL_H
#define DECAYCASCADEITEMMODEL_H

#include <QAbstractItemModel>

class AbstractDataSource;

class DecayCascadeItemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit DecayCascadeItemModel(AbstractDataSource *datasource, QObject *parent = 0);
    ~DecayCascadeItemModel();

    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex & index ) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    
signals:
    
public slots:

private:
    AbstractDataSource *ds;
    
};

#endif // DECAYCASCADEITEMMODEL_H
