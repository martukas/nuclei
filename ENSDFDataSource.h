#ifndef ENSDFDATASOURCE_H
#define ENSDFDATASOURCE_H

#include <QStringList>
#include <QSharedPointer>
#include <QMap>

#include "DataSource.h"

class ENSDFMassChain;

class ENSDFDataSource : public DataSource
{
public:
    explicit ENSDFDataSource();

    virtual int numLevels() const;
    virtual QStringList firstLevel() const;
    virtual QStringList secondLevel(const QString &firstlevel) const;
    virtual QStringList thirdLevel(const QString &firstlevel, const QString &secondlevel) const;

    virtual QSharedPointer<Decay> decay(const QString &firstlevel, const QString &secondlevel,
                                        const QString &thirdlevel) const;

private:
    mutable QMap<QString, ENSDFMassChain*> masschains;
};

#endif // ENSDFDATASOURCE_H
