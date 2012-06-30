#include "Energy.h"

#include <limits>
#include <cmath>

Energy::Energy()
    : e(std::numeric_limits<double>::quiet_NaN())
{
}

Energy::Energy(double energy)
    : e(energy)
{
}

bool Energy::isValid() const
{
    return std::isfinite(e);
}

Energy::operator double() const
{
    return e;
}


QString Energy::toString() const
{
    if (!std::isfinite(e))
        return QString();

    if (e >= 10000.0)
        return QString::number(e / 1.E3) + " MeV";
    return QString::number(e) + " keV";
}

bool operator<(const Energy &left, const Energy &right)
{
    return left.e < right.e;
}

bool operator<(const Energy &left, const double &right)
{
    return left.e < right;
}

bool operator>(const Energy &left, const Energy &right)
{
    return left.e > right.e;
}

bool operator>(const Energy &left, const double &right)
{
    return left.e > right;
}

bool operator==(const Energy &left, const Energy &right)
{
    return qFuzzyCompare(left.e, right.e);
}

QDataStream & operator<<(QDataStream &out, const Energy &energy)
{
    out << energy.e;
    return out;
}

QDataStream & operator>>(QDataStream &in, Energy &energy)
{
    in >> energy.e;
    return in;
}


