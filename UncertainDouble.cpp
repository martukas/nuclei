#include "UncertainDouble.h"

#include <limits>
#include <cmath>
#include <QStringList>

UncertainDouble::UncertainDouble()
    : m_val(std::numeric_limits<double>::quiet_NaN()),
      m_lowerSigma(std::numeric_limits<double>::quiet_NaN()),
      m_upperSigma(std::numeric_limits<double>::quiet_NaN()),
      m_sign(UndefinedSign),
      m_type(UndefinedType)
{
}

UncertainDouble::UncertainDouble(double d, Sign s)
    : m_val(d),
      m_lowerSigma(0.0),
      m_upperSigma(0.0),
      m_sign(s),
      m_type(SymmetricUncertainty)
{
}

UncertainDouble::UncertainDouble(double d, UncertainDouble::Sign s, double symmetricSigma)
    : m_val(d),
      m_lowerSigma(symmetricSigma),
      m_upperSigma(symmetricSigma),
      m_sign(s),
      m_type(SymmetricUncertainty)
{
}

UncertainDouble &UncertainDouble::operator =(const UncertainDouble &other)
{
    if (this != &other) {
        m_val = other.m_val;
        m_lowerSigma = other.m_lowerSigma;
        m_upperSigma = other.m_upperSigma;
        m_sign = other.m_sign;
        m_type = other.m_type;
    }
    return *this;
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

UncertainDouble::UncertaintyType UncertainDouble::uncertaintyType() const
{
    return m_type;
}

UncertainDouble::Sign UncertainDouble::sign() const
{
    return m_sign;
}

void UncertainDouble::setValue(double val, Sign s)
{
    m_val = val;
    m_sign = s;
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

void UncertainDouble::setSign(UncertainDouble::Sign s)
{
    m_sign = s;
}

bool UncertainDouble::hasFiniteValue() const
{
    if (    sign() != UncertainDouble::MagnitudeDefined &&
            sign() != UncertainDouble::SignMagnitudeDefined )
        return false;

    if (    uncertaintyType() == UncertainDouble::SymmetricUncertainty ||
            uncertaintyType() == UncertainDouble::AsymmetricUncertainty ||
            uncertaintyType() == UncertainDouble::Approximately ||
            uncertaintyType() == UncertainDouble::Calculated ||
            uncertaintyType() == UncertainDouble::Systematics )
        return true;
    return false;
}
#include <iostream>
QString UncertainDouble::toString() const
{
    QString signprefix;
    double val = m_val;
    if (m_sign == MagnitudeDefined) {
        signprefix = QString::fromUtf8("± ");
        val = std::abs(val);
    }
    else if (m_sign == UndefinedSign) {
        signprefix = "? ";
        val = std::abs(val);
    }

    switch (m_type) {
    case Systematics:
        return QString("%1%2 (systematics)").arg(signprefix).arg(val).replace('e', "E");
    case Calculated:
        return QString("%1%2 (calculated)").arg(signprefix).arg(val).replace('e', "E");
    case Approximately:
        return QString("~ %1%2").arg(signprefix).arg(val, 0, 'g', 3).replace('e', "E");
    case GreaterEqual:
        return QString(QString::fromUtf8("≥ %1%2")).arg(signprefix).arg(val).replace('e', "E");
    case GreaterThan:
        return QString(QString::fromUtf8("> %1%2")).arg(signprefix).arg(val).replace('e', "E");
    case LessEqual:
        return QString(QString::fromUtf8("≤ %1%2")).arg(signprefix).arg(val).replace('e', "E");
    case LessThan:
        return QString(QString::fromUtf8("< %1%2")).arg(signprefix).arg(val).replace('e', "E");
    case SymmetricUncertainty:
        Q_ASSERT(m_lowerSigma == m_upperSigma);
    case AsymmetricUncertainty:
        Q_ASSERT(std::isfinite(m_upperSigma) && m_upperSigma >= 0.0);
        Q_ASSERT(std::isfinite(m_lowerSigma) && m_lowerSigma >= 0.0);

    {
        // return precise numbers without uncertainty
        if (m_upperSigma == 0.0 && m_lowerSigma == 0.0)
            return QString("%1%2").arg(signprefix).arg(val, 0, 'g', 4).replace('e', "E");

        // determine orders of magnitude to align uncertainty output
        int orderOfValue = std::floor(std::log10(std::abs(val)));
        int orderOfUncert = std::min(std::floor(std::log10(m_lowerSigma)), std::floor(std::log10(m_upperSigma)));

        int precision = 0;
        QString uncertstr;

        // uncertainty counts below 25 of the least significant figure are printed with two digits
        // IF the two digit uncertainty and the value both do not end with 0. Else a one digit unvertainty is printed.

        // for asymmetric uncertainties both values need to be checked.
        if (m_type == AsymmetricUncertainty) {
            double lowerUncertNumber = m_lowerSigma/pow(10.0, orderOfUncert);
            double upperUncertNumber = m_upperSigma/pow(10.0, orderOfUncert);
            if (lowerUncertNumber <= 2.5 && upperUncertNumber <= 2.5 &&
                    (
                        qRound(val * pow(10.0, -orderOfUncert+1)) % 10 != 0 ||
                        qRound(upperUncertNumber*10.0) % 10 != 0 || qRound(lowerUncertNumber*10.0) % 10 != 0
                    )
                ) {
                orderOfUncert--;
                lowerUncertNumber *= 10.0;
                upperUncertNumber *= 10.0;
            }
            uncertstr = QString("(+%1-%2)").arg(qRound(upperUncertNumber)).arg(qRound(lowerUncertNumber));
        }
        // for the symmetric case: check only one value
        else if (m_type == SymmetricUncertainty) {
            double uncertNumber = m_lowerSigma/pow(10.0, orderOfUncert);
            if (uncertNumber <= 2.5 &&
                    (qRound(val * pow(10.0, -orderOfUncert+1)) % 10 != 0 || qRound(uncertNumber*10.0) % 10 != 0)) {
                orderOfUncert--;
                uncertNumber *= 10.0;
            }
            uncertstr = QString("(%1)").arg(qRound(uncertNumber));
        }

        precision = orderOfValue - orderOfUncert;

        QString result;
        if (precision < 0) {
            result = QString("(%1)").arg(QString::number(val, 'g', 0));
        }
        // use standard notation inside ]10,0.01] OR inside ]1000,0.01] if error is < 10
        else if ((qAbs(val) < 10.0 && qAbs(val) >= 0.01) || ((orderOfUncert < 1 && qAbs(val) >= 0.01))) {
            // fix Qt's strange idea of precision...
            int qtprecision = std::abs(val) < 1.0 ? precision+1 : qMax(0, -orderOfUncert);
            if (std::abs(val) < 0.1)
                qtprecision++;

            result = QString::number(val, 'f', qtprecision);
            result.append(uncertstr);
        }
        // use scientific notation for values outside ]10,0.01]
        else {
            result = QString::number(val, 'e', precision);
            result = result.split('e').join(uncertstr + "e");
        }
        result = result.replace('e', "E");
        return signprefix + result;
    }
    default:
        return "undefined";
    }
}

QString UncertainDouble::toText() const
{
    QString result(toString());
    result.replace("(systematics)", "<i>(systematics)</i>");
    result.replace("(calculated)", "<i>(calculated)</i>");
    result.replace("(systematics)", "<i>(systematics)</i>");
    result.replace("<", "&lt;");
    result.replace(">", "&gt;");
    //result.replace(QRegExp("e([-+][0-9][0-9])"), QString::fromUtf8("⋅10<sup>\\1</sup>"));
    return result;
}

UncertainDouble & UncertainDouble::operator*=(double other)
{
    setValue(value() * other);
    if (other >= 0.0 ||
            uncertaintyType() == SymmetricUncertainty ||
            uncertaintyType() == Approximately ||
            uncertaintyType() == Calculated ||
            uncertaintyType() == Systematics
            ) {
        setUncertainty(lowerUncertainty() * other, upperUncertainty() * other, uncertaintyType());
    }
    else { // "other" is always negative in this branch!
        if (uncertaintyType() == AsymmetricUncertainty)
            setUncertainty(upperUncertainty() * other, lowerUncertainty() * other, uncertaintyType());
        else if (uncertaintyType() == LessThan)
            setUncertainty(lowerUncertainty() * other, upperUncertainty() * other, GreaterThan);
        else if (uncertaintyType() == LessEqual)
            setUncertainty(lowerUncertainty() * other, upperUncertainty() * other, GreaterEqual);
        else if (uncertaintyType() == GreaterThan)
            setUncertainty(lowerUncertainty() * other, upperUncertainty() * other, LessThan);
        else if (uncertaintyType() == GreaterEqual)
            setUncertainty(lowerUncertainty() * other, upperUncertainty() * other, LessEqual);
        else
            setUncertainty(lowerUncertainty() * other, upperUncertainty() * other, uncertaintyType());
    }

    return *this;
}

UncertainDouble &UncertainDouble::operator +=(const UncertainDouble &other)
{
    setValue(value() + other.value());
    if (uncertaintyType() == other.uncertaintyType())
        setUncertainty(lowerUncertainty() + other.lowerUncertainty(), upperUncertainty() + other.upperUncertainty(), uncertaintyType());
    else
        setUncertainty(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), UndefinedType);
    return *this;
}

UncertainDouble UncertainDouble::operator +(const UncertainDouble &other)
{
    UncertainDouble result(*this);
    result += other;
    return result;
}

UncertainDouble::operator double() const
{
    return m_val;
}



QDataStream &operator <<(QDataStream &out, const UncertainDouble &u)
{
    out << u.m_val;
    out << u.m_lowerSigma;
    out << u.m_upperSigma;
    out << int(u.m_sign);
    out << int(u.m_type);
    return out;
}


QDataStream &operator >>(QDataStream &in, UncertainDouble &u)
{
    in >> u.m_val;
    in >> u.m_lowerSigma;
    in >> u.m_upperSigma;
    int tmp;
    in >> tmp;
    u.m_sign = UncertainDouble::Sign(tmp);
    in >> tmp;
    u.m_type = UncertainDouble::UncertaintyType(tmp);
    return in;
}
