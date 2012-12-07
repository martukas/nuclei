#ifndef UNCERTAINDOUBLE_H
#define UNCERTAINDOUBLE_H

#include <QString>
#include <QMetaType>
#include <QDataStream>

class UncertainDouble
{
public:
    enum UncertaintyType {
        UndefinedType         = 0x0,
        SymmetricUncertainty  = 0x1,
        AsymmetricUncertainty = 0x2,
        LessThan              = 0x3,
        LessEqual             = 0x4,
        GreaterThan           = 0x5,
        GreaterEqual          = 0x6,
        Approximately         = 0x7,
        Calculated            = 0x8,
        Systematics           = 0x9
    };

    enum Sign {
        UndefinedSign         = 0x0,
        MagnitudeDefined      = 0x1,
        SignDefined           = 0x2,
        SignMagnitudeDefined  = 0x3
    };

    UncertainDouble();
    UncertainDouble(double d, Sign s);
    UncertainDouble(double d, Sign s, double symmetricSigma);

    UncertainDouble & operator=(const UncertainDouble & other);

    double value() const;
    double lowerUncertainty() const;
    double upperUncertainty() const;
    UncertaintyType uncertaintyType() const;
    Sign sign() const;

    void setValue(double val, Sign s = SignMagnitudeDefined);
    void setUncertainty(double lower, double upper, UncertaintyType type);
    void setSymmetricUncertainty(double sigma);
    void setAsymmetricUncertainty(double lowerSigma, double upperSigma);
    void setSign(Sign s);

    bool hasFiniteValue() const;

    QString toString() const;
    QString toText() const; // outputs formatted text

    operator double() const;

    friend QDataStream & operator<<(QDataStream &out, const UncertainDouble &u);
    friend QDataStream & operator>>(QDataStream &in, UncertainDouble &u);

private:
    double m_val;
    double m_lowerSigma, m_upperSigma;
    Sign m_sign;
    UncertaintyType m_type;
};

#endif // UNCERTAINDOUBLE_H
