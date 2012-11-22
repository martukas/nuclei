#include "MixingRatio.h"

#include <limits>
#include <cmath>

MixingRatio::MixingRatio()
    : m_valid(false), m_delta(std::numeric_limits<double>::quiet_NaN()), m_state(UnknownDelta)
{
}

MixingRatio::MixingRatio(double delta, State state)
    : m_valid(true), m_delta(delta), m_state(state)
{
}

bool MixingRatio::isValid() const
{
    return m_valid;
}

QString MixingRatio::toString() const
{
    if (m_state == UnknownDelta)
        return "<i>unknown</i>";

    QString dstr(QString::number(m_delta));
    if (m_state == MagnitudeDefined)
        dstr.prepend(QString::fromUtf8("± "));
    return dstr;
}

double MixingRatio::value() const
{
    return m_delta;
}

MixingRatio::State MixingRatio::state() const
{
    return m_state;
}

bool operator <(const MixingRatio &left, const MixingRatio &right)
{
    return left.m_delta < right.m_delta;
}


bool operator <(const MixingRatio &left, const double &right)
{
    return left.m_delta < right;
}


bool operator >(const MixingRatio &left, const MixingRatio &right)
{
    return left.m_delta > right.m_delta;
}


bool operator >(const MixingRatio &left, const double &right)
{
    return left.m_delta > right;
}


bool operator ==(const MixingRatio &left, const MixingRatio &right)
{
    return qFuzzyCompare(left.m_delta, right.m_delta);
}


MixingRatio::operator double() const
{
    return m_delta;
}


QDataStream &operator <<(QDataStream &out, const MixingRatio &delta)
{
    out << delta.m_valid;
    out << delta.m_delta;
    out << delta.m_state;
    return out;
}


QDataStream &operator >>(QDataStream &in, MixingRatio &delta)
{
    in >> delta.m_valid;
    in >> delta.m_delta;
    int tmp;
    in >> tmp;
    delta.m_state = (MixingRatio::State)tmp;
    return in;
}
