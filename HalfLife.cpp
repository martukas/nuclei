#include "HalfLife.h"

#include <limits>
#include <cmath>
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
    return !std::isnan(sec);
}

double HalfLife::seconds() const
{
    return sec;
}

bool HalfLife::isStable() const
{
    return std::isinf(sec);
}

QString HalfLife::toString() const
{
    QString result;

    if (std::isnan(sec))
        return "";

    if (std::isinf(sec))
        return "stable";

    if (sec > 86400. * 365. * 2.)
        result = QString("%1 a").arg(sec / (86400. * 365.));
    else if (sec > 86400.* 2.)
        result = QString("%1 d").arg(sec / 86400.);
    else if (sec > 3600.* 2.)
        result = QString("%1 h").arg(sec / 3600.);
    else if (sec > 60.* 2.)
        result = QString("%1 m").arg(sec / 60.);

    else if (sec < 1.E-15)
        result = QString("%1 as").arg(sec * 1.E18);
    else if (sec < 1.E-12)
        result = QString("%1 fs").arg(sec * 1.E15);
    else if (sec < 1.E-9)
        result = QString("%1 ps").arg(sec * 1.E12);
    else if (sec < 1.E-6)
        result = QString("%1 ns").arg(sec * 1.E9);
    else if (sec < 1.E-3)
        result = QString::fromUtf8("%1 µs").arg(sec * 1.E6);
    else if (sec < 1.)
        result = QString("%1 ms").arg(sec * 1.E3);

    else
        result = QString("%1 s").arg(sec);

    if (uncert)
        result += " ?";

    return result;
}


