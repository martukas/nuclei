#ifndef ENSDFPARSER_H
#define ENSDFPARSER_H
#include "XDecay.h"
#include "Energy.h"
#include "XNuclide.h"
#include <utility>
#include <memory>

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
    std::vector<ParentRecord> parents;
    NuclideId daughter;
    XDecay::Type decayType;
    BlockIndices block;
    std::string dsid;

    static BasicDecayData from_ensdf(const std::string &header, BlockIndices block);
    std::string to_string() const;
private:
    static XDecay::Type parseDecayType(const std::string &tstring);
};

class ENSDFParser
{
public:
    static const std::list<uint16_t> &aValues();

    explicit ENSDFParser(uint16_t A);
    uint16_t aValue() const;

    const std::list<NuclideId> daughterNuclides() const;
    const std::list< std::pair<std::string, NuclideId> > decays(const NuclideId &daughterNuclide) const;


    XDecay decay(const NuclideId &daughterNuclide, const std::string &decayName) const;
private:

    struct StringSubList {
        StringSubList() : first(0), last(0) {}
        StringSubList(size_t first, size_t last) : first(first), last(last) {}
        void clear() { first=0; last=first; }
        size_t first;
        size_t last;
    };

    static std::list<uint16_t> aList;

    static UncertainDouble parseEnsdfMixing(const std::string &s, const std::string &multipolarity);

    template <typename T> T findNearest(const std::map<Energy, T> &map, const Energy &val, Energy *foundVal = 0) const;
    void insertAdoptedLevelsBlock(std::map<Energy, StringSubList> *adoptblocks, const StringSubList &newblock, char dssym) const;
    std::vector<std::string> extractContinuationRecords(const StringSubList &adoptedblock, const std::list<std::string> &requestedRecords, char typeOfContinuedRecord = 'L') const;

    void parseBlocks();

    const uint16_t a;

    std::vector<std::string> contents;
    std::map<NuclideId, BlockIndices> m_adoptedlevels; // daughter coordignates
    std::map<NuclideId, std::map<std::string, BasicDecayData > > m_decays; // daughter coordinates: (decay name: basic data)
    mutable std::map<NuclideId, std::list<std::string>> m_decayNames; // daughter coordinates: decay

    static double norm(std::string rec, double def_value);
};

#endif // ENSDF_H
