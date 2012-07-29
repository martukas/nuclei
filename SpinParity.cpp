#include "SpinParity.h"
#include <QStringList>

SpinParity::SpinParity()
    : m_valid(false), num(0), denom(1), p(Undefined), weakarg(false)
{
}

SpinParity::SpinParity(unsigned int numerator, unsigned int denominator,
                       Parity parity, bool weakarguments, bool valid,
                       const QString &invalidText)
    : m_valid(valid), num(numerator), denom(denominator), p(parity),
      weakarg(weakarguments), invalidText(invalidText)
{
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
        return invalidText;

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
