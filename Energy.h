#ifndef ENERGY_H
#define ENERGY_H

#include <QString>
#include <QMetaType>
#include <QDataStream>

class Energy
{
public:
    Energy();
    explicit Energy(double energy);

    bool isValid() const;

    QString toString() const;

    Energy & operator=(const Energy &energy);
    Energy & operator=(double energy);

    friend bool operator<(const Energy &left, const Energy &right);
    friend bool operator<(const Energy &left, const double &right);
    friend bool operator>(const Energy &left, const Energy &right);
    friend bool operator>(const Energy &left, const double &right);
    friend bool operator==(const Energy &left, const Energy &right);
    operator double() const;

    friend QDataStream & operator<<(QDataStream &out, const Energy &energy);
    friend QDataStream & operator>>(QDataStream &in, Energy &energy);

private:
    double e;
};

Q_DECLARE_METATYPE(Energy)


#endif // ENERGY_H
