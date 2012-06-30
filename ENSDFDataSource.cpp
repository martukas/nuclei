#include "ENSDFDataSource.h"

#include "ENSDFMassChain.h"


ENSDFDataSource::ENSDFDataSource()
{
}

int ENSDFDataSource::numLevels() const
{
    return 3;
}

QStringList ENSDFDataSource::firstLevel() const
{
    return ENSDFMassChain::aValues();
}

QStringList ENSDFDataSource::secondLevel(const QString &firstlevel) const
{
    ENSDFMassChain* mc = 0;
    if (masschains.contains(firstlevel)) {
        mc = masschains.value(firstlevel);
    }
    else {
        mc = new ENSDFMassChain(firstlevel);
        masschains.insert(firstlevel, mc);
    }

    return mc->daughterNuclides();
}

QStringList ENSDFDataSource::thirdLevel(const QString &firstlevel, const QString &secondlevel) const
{
    return masschains.value(firstlevel)->decays(secondlevel);
}

QSharedPointer<Decay> ENSDFDataSource::decay(const QString &firstlevel, const QString &secondlevel, const QString &thirdlevel) const
{
    return masschains.value(firstlevel)->decay(secondlevel, thirdlevel);
}
