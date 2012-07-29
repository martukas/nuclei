#ifndef SPINPARITY_H
#define SPINPARITY_H

#include <QString>

class SpinParity
{
public:
    enum Parity {
        Plus,
        Minus,
        Undefined
    };

    SpinParity();

    SpinParity(unsigned int numerator, unsigned int denominator,
               Parity parity = Undefined, bool weakarguments = false,
               bool valid = true, const QString &invalidText = QString());

    bool isValid() const;

    int doubledSpin() const;
    QString toString() const;

private:
    bool m_valid;
    unsigned int num;
    unsigned int denom;
    Parity p;
    bool weakarg;
    QString invalidText;
};

#endif // SPINPARITY_H
