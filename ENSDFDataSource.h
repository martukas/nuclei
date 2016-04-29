#ifndef ENSDFDATASOURCE_H
#define ENSDFDATASOURCE_H

#include <QStringList>
#include <QSharedPointer>
#include <QMap>
#include <QMetaType>
#include <QVariant>
#include <QMutex>

#include "AbstractDataSource.h"

class ENSDFParser;

class ENSDFTreeItem : public AbstractTreeItem
{
public:
    explicit ENSDFTreeItem(ItemType type = UnknownType, AbstractTreeItem *parent = 0);
    explicit ENSDFTreeItem(ItemType type, const QList<QVariant> &data, NuclideId id, bool isdecay, AbstractTreeItem *parent = 0);
    virtual ~ENSDFTreeItem();

    friend QDataStream & operator<<(QDataStream &out, const ENSDFTreeItem &treeitem);
    friend QDataStream & operator>>(QDataStream &in, ENSDFTreeItem &treeitem);
};

Q_DECLARE_METATYPE(ENSDFTreeItem)

class ENSDFDataSource : public AbstractDataSource
{
    Q_OBJECT

public:
    explicit ENSDFDataSource(QObject *parent = 0);
    virtual ~ENSDFDataSource();

    virtual AbstractTreeItem * rootItem() const;

    virtual DecaySchemePtr decay(const AbstractTreeItem *item) const;

public slots:
    void deleteDatabaseAndCache();
    void deleteCache();

private:
    QList<uint16_t> getAvailableDataFileNumbers() const;

    QString cachePath;

    static const quint32 magicNumber;
    static const quint32 cacheVersion;

    bool loadENSDFCache();
    void createENSDFCache();

    ENSDFTreeItem *root;

    mutable ENSDFParser *mccache;
    mutable QMutex m;
};

#endif // ENSDFDATASOURCE_H
