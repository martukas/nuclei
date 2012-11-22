#include "UncertainDouble.h"

#include <limits>
#include <cmath>
#include <QStringList>

UncertainDouble::UncertainDouble()
    : m_val(std::numeric_limits<double>::quiet_NaN()),
      m_lowerSigma(std::numeric_limits<double>::quiet_NaN()),
      m_upperSigma(std::numeric_limits<double>::quiet_NaN()),
      m_type(Undefined)
{
}

UncertainDouble::UncertainDouble(double d)
    : m_val(d),
      m_lowerSigma(0.0),
      m_upperSigma(0.0),
      m_type(Undefined)
{
}

double UncertainDouble::value() const
{
    return m_val;
}

double UncertainDouble::lowerUncertainty() const
{
    return m_lowerSigma;
}

double UncertainDouble::upperUncertainty() const
{
    return m_upperSigma;
}

void UncertainDouble::setValue(double val)
{
    m_val = val;
}

void UncertainDouble::setUncertainty(double lower, double upper, UncertainDouble::UncertaintyType type)
{
    m_lowerSigma = lower;
    m_upperSigma = upper;
    m_type = type;
}

void UncertainDouble::setSymmetricUncertainty(double sigma)
{
    setUncertainty(sigma, sigma, SymmetricUncertainty);
}

void UncertainDouble::setAsymmetricUncertainty(double lowerSigma, double upperSigma)
{
    setUncertainty(lowerSigma, upperSigma, AsymmetricUncertainty);
}

QString UncertainDouble::toString() const
{
    switch (m_type) {
    case Systematics:
        return QString("%1 (systematics)").arg(m_val);
    case Calculated:
        return QString("%1 (calculated)").arg(m_val);
    case Approximately:
        return QString("~ %1").arg(m_val);
    case GreaterEqual:
        return QString(QString::fromUtf8("≥ %1")).arg(m_val);
    case GreaterThan:
        return QString(QString::fromUtf8("> %1")).arg(m_val);
    case LessEqual:
        return QString(QString::fromUtf8("≤ %1")).arg(m_val);
    case LessThan:
        return QString(QString::fromUtf8("< %1")).arg(m_val);
    case SymmetricUncertainty:
        Q_ASSERT(m_lowerSigma == m_upperSigma);
    case AsymmetricUncertainty:
        Q_ASSERT(std::isfinite(m_upperSigma) && m_upperSigma > 0.0);
        Q_ASSERT(std::isfinite(m_lowerSigma) && m_lowerSigma > 0.0);

    {
        // determine orders of magnitude to align uncertainty output
        int orderOfValue = std::floor(std::log10(m_val));
        int orderOfUncert = std::min(std::floor(std::log10(m_lowerSigma)), std::floor(std::log10(m_upperSigma)));

        int precision = 0;
        QString uncertstr;

        // uncertainty counts below 25 of the least significant figure are printed with two digits. else one.
        // for asymmetric uncertainties both values need to be checked.
        if (m_type == AsymmetricUncertainty) {
            double lowerUncertNumber = m_lowerSigma/pow(10.0, orderOfUncert);
            double upperUncertNumber = m_upperSigma/pow(10.0, orderOfUncert);
            if (lowerUncertNumber <= 2.5 && upperUncertNumber <= 2.5) {
                orderOfUncert--;
                lowerUncertNumber *= 10.0;
                upperUncertNumber *= 10.0;
            }
            uncertstr = QString("(+%1-%2)").arg(qRound(upperUncertNumber)).arg(qRound(lowerUncertNumber));
        }
        else if (m_type == SymmetricUncertainty) {
            double uncertNumber = m_lowerSigma/pow(10.0, orderOfUncert);
            if (uncertNumber <= 2.5) {
                orderOfUncert--;
                uncertNumber *= 10.0;
            }
            uncertstr = QString("(%1)").arg(qRound(uncertNumber));
        }
        precision = orderOfValue - orderOfUncert;

        // use scientific notation for values outside ]10,0.1]
        QString result;
        if (precision < 0) {
            result = QString("(%1)").arg(QString::number(m_val, 'g', 0));
        }
        else if (m_val < 10.0 && m_val >= 0.1) {
            result = QString::number(m_val, 'f', precision);
            result.append(uncertstr);
        }
        else {
            result = QString::number(m_val, 'e', precision);
            result = result.split('e').join(uncertstr + "e");
        }
        return result;
    }
    default:
        return "?";
    }
}

UncertainDouble::operator double() const
{
    return m_val;
}



QDataStream &operator <<(QDataStream &out, const UncertainDouble &u)
{
    out << u.m_val;
    out << u.m_lowerSigma;
    return out;
}


QDataStream &operator >>(QDataStream &in, UncertainDouble &u)
{
    in >> u.m_val;
    in >> u.m_lowerSigma;
    return in;
}
