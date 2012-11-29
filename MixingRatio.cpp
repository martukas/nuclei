#include "MixingRatio.h"

#include <limits>
#include <cmath>

MixingRatio::MixingRatio()
    : m_valid(false)
{
}

MixingRatio::MixingRatio(UncertainDouble delta)
    : m_valid(true), m_delta(delta)
{
}

bool MixingRatio::isValid() const
{
    return m_valid;
}

QString MixingRatio::toString() const
{
    return m_delta.toString();
}

UncertainDouble MixingRatio::value() const
{
    return m_delta;
}

bool operator <(const MixingRatio &left, const MixingRatio &right)
{
    return left.m_delta < right.m_delta;
}


bool operator <(const MixingRatio &left, const UncertainDouble &right)
{
    return left.m_delta < right;
}


bool operator >(const MixingRatio &left, const MixingRatio &right)
{
    return left.m_delta > right.m_delta;
}


bool operator >(const MixingRatio &left, const UncertainDouble &right)
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
