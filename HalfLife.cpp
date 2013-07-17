#include "HalfLife.h"

#include <limits>
#include <cmath>
#include <boost/math/special_functions/fpclassify.hpp>
#include <QStringList>
#include <QLocale>

HalfLife::HalfLife()
    : sec(std::numeric_limits<double>::quiet_NaN()), uncert(false)
{
}

HalfLife::HalfLife(double seconds, bool uncertain)
    : sec(seconds), uncert(uncertain)
{
}

bool HalfLife::isValid() const
{
    return !boost::math::isnan(sec);
}

double HalfLife::seconds() const
{
    return sec;
}

bool HalfLife::isStable() const
{
    return boost::math::isinf(sec);
}

QString HalfLife::toString() const
{
    return secsToString(sec, uncert);
}

QString HalfLife::secsToString(double secs, bool tagUncertain)
{
    if (boost::math::isnan(secs))
        return "";

    if (boost::math::isinf(secs))
        return "stable";

    QString result;

    if (secs > 86400. * 365. * 2.)
        result = QString("%1 a").arg(secs / (86400. * 365.));
    else if (secs > 86400.* 2.)
        result = QString("%1 d").arg(secs / 86400.);
    else if (secs > 3600.* 2.)
        result = QString("%1 h").arg(secs / 3600.);
    else if (secs > 60.* 2.)
        result = QString("%1 m").arg(secs / 60.);

    else if (secs < 1.E-15)
        result = QString("%1 as").arg(secs * 1.E18);
    else if (secs < 1.E-12)
        result = QString("%1 fs").arg(secs * 1.E15);
    else if (secs < 1.E-9)
        result = QString("%1 ps").arg(secs * 1.E12);
    else if (secs < 1.E-6)
        result = QString("%1 ns").arg(secs * 1.E9);
    else if (secs < 1.E-3)
        result = QString::fromUtf8("%1 µs").arg(secs * 1.E6);
    else if (secs < 1.)
        result = QString("%1 ms").arg(secs * 1.E3);

    else
        result = QString("%1 s").arg(secs);

    if (tagUncertain)
        result += " ?";

    return result;
}

bool HalfLife::operator >(const HalfLife &right) const
{
    return sec > right.sec;
}

bool HalfLife::operator >=(const HalfLife &right) const
{
    return sec >= right.sec;
}

bool HalfLife::operator <(const HalfLife &right) const
{
    return sec < right.sec;
}


