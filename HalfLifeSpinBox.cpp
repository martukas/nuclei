#include "HalfLifeSpinBox.h"

#include <limits>
#include <cmath>

#include "HalfLife.h"

HalfLifeSpinBox::HalfLifeSpinBox(QWidget *parent) :
    QDoubleSpinBox(parent)
{
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

double HalfLifeSpinBox::validateAndInterpret(QString &input, int &pos, QValidator::State &state) const
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
        QRegExp unitre("^(.+)\\s*([a-zA-Z]+)$");
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
