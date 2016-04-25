#ifndef ENERGY_H
#define ENERGY_H

#include <string>
#include "UncertainDouble.h"

class Energy
{
public:
    Energy();
    explicit Energy(double energy, UncertainDouble::Sign s);

    static Energy from_nsdf(std::string record);

    bool isValid() const;

    std::string to_string() const;

    Energy & operator=(const Energy &energy);

    friend bool operator<(const Energy &left, const Energy &right);
    friend bool operator<(const Energy &left, const double &right);
    friend bool operator>(const Energy &left, const Energy &right);
    friend bool operator>(const Energy &left, const double &right);
    friend bool operator==(const Energy &left, const Energy &right);
    operator double() const;

    Energy operator-(Energy d);

private:
    UncertainDouble e;
};

#endif // ENERGY_H
