#include "HalfLifeSpinBox.h"

#include <limits>
#include <cmath>

#include "HalfLife.h"

HalfLifeSpinBox::HalfLifeSpinBox(QWidget *parent) :
    QDoubleSpinBox(parent)
{
    setDecimals(20);
}

QString HalfLifeSpinBox::textFromValue(double value) const
{
    return HalfLife::secsToString(value, false);
}

double HalfLifeSpinBox::valueFromText(const QString &text) const
{
    QString copy(text);
    QValidator::State state = QValidator::Acceptable;
    int pos = 0;
    return validateAndInterpret(copy, pos, state);
}

QValidator::State HalfLifeSpinBox::validate(QString &text, int &pos) const
{
    QValidator::State state;
    validateAndInterpret(text, pos, state);
    return state;
}

void HalfLifeSpinBox::stepBy(int steps)
{
    for (int i=std::abs(steps); i>0; i--) {
        double val = value();
        double step = 1.0;
        // log10 stepping for values below 1 min.
        if (val <= 10.0) {
            double logval = std::log10(val);
            double exponent = std::floor(logval);
            if (steps < 0 && logval - exponent < std::log10(1.1))
                exponent--;
            step = pow(10.0, exponent);
        }
        // special stepping for longer values
        else {
            if ((steps > 0 && val >= 365. * 86400.) || val > 365. * 86400.)
                step = 365. * 86400.;
            else if ((steps > 0 && val >= 86400.) || val > 86400.)
                step = 86400.;
            else if ((steps > 0 && val >= 3600.) || val > 3600.)
                step = 3600.;
            else if ((steps > 0 && val >= 600.) || val > 600.)
                step = 600.;
            else if ((steps > 0 && val >= 60.) || val > 60.)
                step = 60.;
            else
                step = 10.;
        }
        setSingleStep(step);
        QDoubleSpinBox::stepBy(steps > 0 ? 1 : -1);
    }
}

void HalfLifeSpinBox::setDecimals(int prec)
{
    QDoubleSpinBox::setDecimals(20);
}

double HalfLifeSpinBox::validateAndInterpret(QString &input, int &, QValidator::State &state) const
{
    QLocale locale;
    double num = 0.0;
    double factor = 1.0;

    if (input.isEmpty()) {
        state = QValidator::Intermediate;
    }
    else if (input.size() == 1 && input.at(0) == locale.decimalPoint()) {
        state = QValidator::Intermediate;
    }
    else {
        QString copy(input);

        // find unit suffix and remove it from copy
        QRegExp unitre("^([^a-zA-Z]+)\\s*([a-zA-Z]+)$");
        QString unit;
        if (unitre.exactMatch(copy)) {
            unit = unitre.capturedTexts().value(2);
            copy = unitre.capturedTexts().value(1);
        }

        factor = unitToFactor(unit);

        bool ok = false;
        num = locale.toDouble(copy.remove(locale.groupSeparator()), &ok);

        if (!ok)
            state = QValidator::Invalid;
        else if (num < 0.0)
            state = QValidator::Invalid;
        else if (!std::isfinite(factor))
            state = QValidator::Intermediate;
        else
            state = QValidator::Acceptable;
    }

    if (state != QValidator::Acceptable)
        num = 0.0;

    return num * factor;
}

double HalfLifeSpinBox::unitToFactor(const QString &unit)
{
    if (unit.isEmpty())
        return std::numeric_limits<double>::quiet_NaN();

    if (unit == "a" || unit == "year" || unit == "years")
        return 86400. * 365.;

    if (unit == "d" || unit == "day" || unit == "days")
        return 86400.;

    if (unit == "h" || unit == "hour" || unit == "hours")
        return 3600.;

    if (unit == "m" || unit == "min" || unit == "mins" || unit == "minute" || unit == "minutes")
        return 60.;

    if (unit == "s" || unit == "sec" || unit == "secs" || unit == "second" || unit == "seconds")
        return 1.;

    if (unit == "ms")
        return 1.E-3;

    if (unit == QString::fromUtf8("µs") || unit == "us" || unit == "u")
        return 1.E-6;

    if (unit == "ns" || unit == "n")
        return 1.E-9;

    if (unit == "ps" || unit == "p")
        return 1.E-12;

    if (unit == "fs" || unit == "f")
        return 1.E-15;

    if (unit == "as")
        return 1.E-18;

    return std::numeric_limits<double>::quiet_NaN();
}
