#ifndef ENSDFPARSER_H
#define ENSDFPARSER_H
#include <QStringList>
#include <QSharedPointer>
#include <QPair>
#include "Decay.h"
#include "Energy.h"
#include "Nuclide.h"

class HalfLife;

class ENSDFParser
{
public:

    explicit ENSDFParser(unsigned int A);

    static const QList<unsigned int> &aValues();

    unsigned int aValue() const;
    const QList<Nuclide::Coordinates> daughterNuclides() const;

    /**
     * @brief Lists with basic decay path data for a given daughter
     * @param daughterNuclide daughter
     * @return List of (Decay String, Parent Nucleus Coordinate) pairs
     */
    const QList< QPair<QString, Nuclide::Coordinates> > decays(const Nuclide::Coordinates &daughterNuclide) const;

    /**
     * @brief decay
     * @param daughterNuclideName
     * @param decayName
     * @param adoptedLevelMaxDifference Maximal acceptable difference between energy levels (in eV) in decay and adopted level blocks
     * @param gammaMaxDifference Maximal difference between the energy of gammas in decay records and adopted levels
     * @return
     */
    QSharedPointer<Decay> decay(const Nuclide::Coordinates &daughterNuclide, const QString &decayName) const;

private:
    typedef QPair<int,int> BlockIndices; // [startidx, size]

    struct ParentRecord {
        Nuclide::Coordinates nuclide;
        Energy energy;
        HalfLife hl;
        SpinParity spin;

    };

    struct BasicDecayData {
        QList<ParentRecord> parents;
        Nuclide::Coordinates daughter;
        Decay::Type decayType;
        BlockIndices block;
        QString dsid;
    };

    struct StringSubList {
        StringSubList() : first(QStringList::const_iterator()), last(first) {}
        StringSubList(QStringList::const_iterator first, QStringList::const_iterator last) : first(first), last(last) {}
        void clear() { first=QStringList::const_iterator(); last=first; }
        QStringList::const_iterator first;
        QStringList::const_iterator last;
    };

    static QList<unsigned int> aList;

    static QString nuclideToNucid(Nuclide::Coordinates nuclide);
    static Nuclide::Coordinates nucidToNuclide(const QString &nucid);
    static Decay::Type parseDecayType(const QString &tstring);
    static Energy parseEnsdfEnergy(const QString &estr);
    static HalfLife parseHalfLife(const QString &hlstr);
    static SpinParity parseSpinParity(const QString &sstr);
    static UncertainDouble parseEnsdfMixing(const QString &s, const QString &multipolarity);
    static ParentRecord parseParentRecord(const QString &precstr);
    static UncertainDouble parseMoment(const QString &s);

    static UncertainDouble parseUncertainty(const QString &value, const QString &uncertaintyString);
    static double getUncertainty(const QString value, unsigned int stdUncertainty);

    template <typename T> T findNearest(const QMap<Energy, T> &map, const Energy &val, Energy *foundVal = 0) const;
    static void insertAdoptedLevelsBlock(QMap<Energy, StringSubList> *adoptblocks, const StringSubList &newblock, char dssym);
    static QStringList extractContinuationRecords(const StringSubList &adoptedblock, const QStringList &requestedRecords, char typeOfContinuedRecord = 'L');

    void parseBlocks();

    const unsigned int a;

    QStringList contents;
    QMap<Nuclide::Coordinates, BlockIndices> m_adoptedlevels; // daughter coordignates: [startidx, size]
    QMap<Nuclide::Coordinates, QMap<QString, BasicDecayData > > m_decays; // daughter coordinates: (decay name: basic data)
    mutable QMap<Nuclide::Coordinates, QStringList> m_decayNames; // daughter coordinates: decay
};

#endif // ENSDF_H
