#include "SpinParity.h"
#include <QStringList>

SpinParity::SpinParity()
    : m_valid(false), num(0), denom(1), p(Undefined), weakarg(false)
{
}

SpinParity::SpinParity(unsigned int numerator, unsigned int denominator,
                       Parity parity, bool weakarguments)
    : m_valid(true), num(numerator), denom(denominator), p(parity), weakarg(weakarguments)
{
}

SpinParity::SpinParity(const QString &ensdfData)
    : m_valid(false), num(0), denom(1), p(Undefined), weakarg(false)
{
    QString spstr(ensdfData.trimmed());
    if (spstr.contains('('))
        weakarg = true;
    spstr.remove('(').remove(')');
    spstr = spstr.trimmed();
    if (spstr.right(1) == "+")
        p = Plus;
    else if (spstr.right(1) == "-")
        p = Minus;
    spstr.remove('+').remove('-');
    QStringList fract(spstr.split('/'));
    if (!fract.isEmpty()) {
        num = fract.at(0).toUInt();
        if (fract.size() > 1)
            denom = fract.at(1).toUInt();
        if (denom == 0)
            denom = 1;
        m_valid = true;
    }
}

bool SpinParity::isValid() const
{
    return m_valid;
}

int SpinParity::doubledSpin() const
{
    return (denom == 1) ? num * 2 : num;
}

QString SpinParity::toString() const
{
    if (!m_valid)
        return "";

    QString sign;
    switch (p) {
    case Plus:
        sign = "+";
        break;
    case Minus:
        sign = "-";
        break;
    default:
        break;
    }

    QString result = QString::number(num);

    if (denom != 1)
        result += QString("/%1").arg(denom);

    result += sign;

    if (weakarg)
        result = "(" + result + ")";

    return result;
}
