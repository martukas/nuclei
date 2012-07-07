#ifndef ENSDFDATASOURCE_H
#define ENSDFDATASOURCE_H

#include <QStringList>
#include <QSharedPointer>
#include <QMap>
#include <QMetaType>
#include <QVariant>

#include "AbstractDataSource.h"

class ENSDFParser;

class ENSDFTreeItem : public AbstractTreeItem
{
public:
    explicit ENSDFTreeItem(AbstractTreeItem *parent = 0);
    explicit ENSDFTreeItem(const QList<QVariant> &data, unsigned int A, bool isdecay, AbstractTreeItem *parent = 0);
    virtual ~ENSDFTreeItem();

    unsigned int A() const;

    friend QDataStream & operator<<(QDataStream &out, const ENSDFTreeItem &treeitem);
    friend QDataStream & operator>>(QDataStream &in, ENSDFTreeItem &treeitem);

private:
    unsigned int m_A;
};

Q_DECLARE_METATYPE(ENSDFTreeItem)

class ENSDFDataSource : public AbstractDataSource
{
public:
    explicit ENSDFDataSource(QObject *parent = 0);
    virtual ~ENSDFDataSource();

    virtual AbstractTreeItem * rootItem() const;

    virtual QSharedPointer<Decay> decay(const AbstractTreeItem *item) const;

private:
    static const quint32 magicNumber;
    static const quint32 cacheVersion;

    bool loadENSDFCache();
    void createENSDFCache();

    ENSDFTreeItem *root;

    mutable ENSDFParser *mccache;
};

#endif // ENSDFDATASOURCE_H
