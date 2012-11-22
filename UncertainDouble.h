#ifndef UNCERTAINDOUBLE_H
#define UNCERTAINDOUBLE_H

#include <QString>
#include <QMetaType>
#include <QDataStream>

class UncertainDouble
{
public:
    enum UncertaintyType {
        Undefined,
        SymmetricUncertainty,
        AsymmetricUncertainty,
        LessThan,
        LessEqual,
        GreaterThan,
        GreaterEqual,
        Approximately,
        Calculated,
        Systematics
    };

    UncertainDouble();
    UncertainDouble(double d);

    double value() const;
    double lowerUncertainty() const;
    double upperUncertainty() const;

    void setValue(double val);
    void setUncertainty(double lower, double upper, UncertaintyType type);
    void setSymmetricUncertainty(double sigma);
    void setAsymmetricUncertainty(double lowerSigma, double upperSigma);

    QString toString() const;

    operator double() const;

    friend QDataStream & operator<<(QDataStream &out, const UncertainDouble &u);
    friend QDataStream & operator>>(QDataStream &in, UncertainDouble &u);

private:
    double m_val;
    double m_lowerSigma, m_upperSigma;
    UncertaintyType m_type;
};

#endif // UNCERTAINDOUBLE_H
