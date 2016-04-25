#ifndef ENSDFPARSER_H
#define ENSDFPARSER_H
#include <QStringList>
#include <QSharedPointer>
#include <QPair>
#include "Decay.h"
#include "Energy.h"
#include "Nuclide.h"

#include <utility>

class HalfLife;

typedef std::pair<int,int> BlockIndices; // [startidx, size]

struct ParentRecord {
    NuclideId nuclide;
    Energy energy;
    HalfLife hl;
    SpinParity spin;

    static ParentRecord from_ensdf(const std::string &precstr);
    std::string to_string() const;
};

struct BasicDecayData {
    QList<ParentRecord> parents;
    NuclideId daughter;
    Decay::Type decayType;
    BlockIndices block;
    std::string dsid;

    static BasicDecayData from_ensdf(const std::string &header, BlockIndices block);
    std::string to_string() const;
private:
    static Decay::Type parseDecayType(const std::string &tstring);
};

class ENSDFParser
{
public:
    static const QList<unsigned int> &aValues();

    explicit ENSDFParser(unsigned int A);
    unsigned int aValue() const;

    const std::list<NuclideId> daughterNuclides() const;
    const std::list< std::pair<QString, NuclideId> > decays(const NuclideId &daughterNuclide) const;


    QSharedPointer<Decay> decay(const NuclideId &daughterNuclide, const std::string &decayName) const;
private:

    struct StringSubList {
        StringSubList() : first(0), last(0) {}
        StringSubList(size_t first, size_t last) : first(first), last(last) {}
        void clear() { first=0; last=first; }
        size_t first;
        size_t last;
    };

    static QList<unsigned int> aList;

    static UncertainDouble parseEnsdfMixing(const QString &s, const QString &multipolarity);

    template <typename T> T findNearest(const QMap<Energy, T> &map, const Energy &val, Energy *foundVal = 0) const;
    void insertAdoptedLevelsBlock(QMap<Energy, StringSubList> *adoptblocks, const StringSubList &newblock, char dssym) const;
    std::vector<std::string> extractContinuationRecords(const StringSubList &adoptedblock, const std::list<std::string> &requestedRecords, char typeOfContinuedRecord = 'L') const;

    void parseBlocks();

    const unsigned int a;

    std::vector<std::string> contents;
    std::map<NuclideId, BlockIndices> m_adoptedlevels; // daughter coordignates
    std::map<NuclideId, std::map<std::string, BasicDecayData > > m_decays; // daughter coordinates: (decay name: basic data)
    mutable std::map<NuclideId, std::list<std::string>> m_decayNames; // daughter coordinates: decay
};

#endif // ENSDF_H
