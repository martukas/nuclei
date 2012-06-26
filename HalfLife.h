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

private:
    double sec;
    bool uncert;
};

#endif // HALFLIFE_H
