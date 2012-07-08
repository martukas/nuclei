#ifndef HALFLIFE_H
#define HALFLIFE_H

#include <QString>

class HalfLife
{
public:
    HalfLife();
    HalfLife(double seconds, bool uncertain = false);

    bool isValid() const;
    double seconds() const;
    bool isStable() const;
    QString toString() const;
    static QString secsToString(double secs, bool tagUncertain);

    bool operator>(const HalfLife &right) const;
    bool operator<(const HalfLife &right) const;

private:
    double sec;
    bool uncert;
};

#endif // HALFLIFE_H
