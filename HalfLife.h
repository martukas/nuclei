#ifndef HALFLIFE_H
#define HALFLIFE_H

#include <string>
#include <map>
#include "UncertainDouble.h"

class HalfLife
{
public:
    HalfLife();
    HalfLife(double seconds);

    static HalfLife from_ensdf(std::string record);

    bool isValid() const;
    double seconds() const;
    bool isStable() const;
    std::string to_string(bool with_uncert = true) const;

    bool operator>(const HalfLife &right) const;
    bool operator>=(const HalfLife &right) const;
    bool operator<(const HalfLife &right) const;

private:
    UncertainDouble time_;
    std::string     units_;


    static const std::map<std::string, double> known_units_;
    static std::map<std::string, double> init_units();

    static std::string preferred_units(double from);
};

#endif // HALFLIFE_H
