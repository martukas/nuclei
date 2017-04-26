#pragma once

#include <QStringList>
#include <QSharedPointer>
#include <QMap>
#include <QMetaType>
#include <QVariant>
#include <QMutex>
#include <QDir>

#include "ENSDFTreeItem.h"

class ENSDFParser;

class ENSDFDataSource : public QObject
{
    Q_OBJECT

public:
    explicit ENSDFDataSource(QObject *parent = 0);
    virtual ~ENSDFDataSource();

    virtual ENSDFTreeItem * rootItem() const;

    virtual DecayScheme decay(const ENSDFTreeItem *item) const;

public slots:
    void deleteDatabaseAndCache();
    void deleteCache();

private:
    QList<uint16_t> getAvailableDataFileNumbers();

    QString cachePath;
    QString defaultPath;
    QDir dest;

    static const quint32 magicNumber;
    static const quint32 cacheVersion;

    bool loadENSDFCache();
    void createENSDFCache();

    ENSDFTreeItem *root;

    mutable ENSDFParser *mccache;
    mutable QMutex m;
};
