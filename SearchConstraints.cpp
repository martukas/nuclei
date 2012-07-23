#include "SearchConstraints.h"
#include <limits>
#include <cmath>

SearchConstraints::SearchConstraints()
    : valid(false),
      minA(0), maxA(std::numeric_limits<unsigned int>::max()),
      minGammaIntensity(std::numeric_limits<double>::quiet_NaN()),
      minMu(std::numeric_limits<double>::quiet_NaN()),
      minQ(std::numeric_limits<double>::quiet_NaN()),
      skipUnknownMu(false), skipUnknownQ(false),
      muAndQORCombined(false),
      minA22(std::numeric_limits<double>::quiet_NaN()),
      minA24(std::numeric_limits<double>::quiet_NaN()),
      minA42(std::numeric_limits<double>::quiet_NaN()),
      minA44(std::numeric_limits<double>::quiet_NaN()),
      skipUnknownAnisotropies(false),
      anisotropiesORCombined(false)
{
}

QStringList SearchConstraints::toStringList() const
{
    QStringList r;

    r.append(QString("%1 < A < %2").arg(minA).arg(maxA));
    if (minParentHl.isValid())
        r.append("Min. parent hl: " + minParentHl.toString());
    if (maxParentHl.isValid())
        r.append("Max. parent hl: " + maxParentHl.toString());
    if (std::isfinite(minGammaIntensity))
        r.append(QString("Min. gamma intensity: %1 %").arg(minGammaIntensity));
    if (minLevelHl.isValid())
        r.append("Min. intermediate hl: " + minLevelHl.toString());
    if (maxLevelHl.isValid())
        r.append("Max. intermediate hl: " + maxLevelHl.toString());
    if (std::isfinite(minMu))
        r.append(QString::fromUtf8("Min. µ: %1%2").arg(minMu).arg(skipUnknownMu ? " (skip unknown)" : ""));
    if (std::isfinite(minQ))
        r.append(QString::fromUtf8("Min. Q: %1%2").arg(minQ).arg(skipUnknownQ ? " (skip unknown)" : ""));
    r.append(QString::fromUtf8("Logical combination of µ and Q: ").arg(muAndQORCombined ? "OR" : "AND"));
    if (std::isfinite(minA22))
        r.append(QString("Min. A22: %1").arg(minA22));
    if (std::isfinite(minA24))
        r.append(QString("Min. A24: %1").arg(minA24));
    if (std::isfinite(minA42))
        r.append(QString("Min. A42: %1").arg(minA42));
    if (std::isfinite(minA44))
        r.append(QString("Min. A44: %1").arg(minA44));
    if (skipUnknownAnisotropies)
        r.append("Skipping unknown anisotropies");
    r.append(QString("Logical combination of anisotropies: %1").arg(anisotropiesORCombined ? "OR" : "AND"));

    return r;
}

QDataStream & operator <<(QDataStream &out, const SearchConstraints &c)
{
    out << c.valid << c.minA << c.maxA << c.minParentHl.seconds() << c.maxParentHl.seconds();
    out << c.minGammaIntensity;

    out << c.minLevelHl.seconds() << c.maxLevelHl.seconds() << c.minMu << c.minQ;

    out << c.skipUnknownMu << c.skipUnknownQ << c.muAndQORCombined;

    out << c.minA22 << c.minA24 << c.minA42 << c.minA44;

    out << c.skipUnknownAnisotropies;
    out << c.anisotropiesORCombined;

    return out;
}


QDataStream &operator >>(QDataStream &in, SearchConstraints &c)
{
    double tmp;
    in >> c.valid >> c.minA >> c.maxA;
    in >> tmp;
    c.minParentHl = HalfLife(tmp);
    in >> tmp;
    c.maxParentHl = HalfLife(tmp);
    in >> c.minGammaIntensity;
    in >> tmp;
    c.minLevelHl = HalfLife(tmp);
    in >> tmp;
    c.maxLevelHl = HalfLife(tmp);
    in >> c.minMu >> c.minQ;
    in >> c.skipUnknownMu >> c.skipUnknownQ >> c.muAndQORCombined;
    in >> c.minA22 >> c.minA24 >> c.minA42 >> c.minA44;
    in >> c.skipUnknownAnisotropies;
    in >> c.anisotropiesORCombined;

    return in;
}
