#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QStringList>

class Decay;

class DataSource
{
public:
    explicit DataSource() {};

    virtual int numLevels() const = 0;
    virtual QStringList firstLevel() const = 0;
    virtual QStringList secondLevel(const QString &firstlevel) const = 0;
    virtual QStringList thirdLevel(const QString &firstlevel, const QString &secondlevel) const = 0;

    virtual QSharedPointer<Decay> decay(const QString &firstlevel, const QString &secondlevel,
                                        const QString &thirdlevel) const = 0;
};

#endif // DATASOURCE_H
