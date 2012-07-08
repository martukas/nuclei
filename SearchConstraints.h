#ifndef SEARCHCONSTRAINTS_H
#define SEARCHCONSTRAINTS_H

#include "HalfLife.h"
#include <QMetaType>

class SearchConstraints
{
public:
    SearchConstraints();

    bool valid;

    unsigned int minA, maxA;
    HalfLife minParentHl, maxParentHl;

    double minGammaIntensity;

    HalfLife minLevelHl, maxLevelHl;
    double minMu;
    double minQ;

    double minA22, minA24, minA42, minA44;

    friend QDataStream & operator<<(QDataStream &out, const SearchConstraints &c);
    friend QDataStream & operator>>(QDataStream &in, SearchConstraints &c);
};

Q_DECLARE_METATYPE(SearchConstraints)

#endif // SEARCHCONSTRAINTS_H
