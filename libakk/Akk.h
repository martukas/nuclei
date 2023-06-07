#ifndef AKK_H
#define AKK_H

#include "akk_global.h"

class AkkPrivate;

class AKKSHARED_EXPORT Akk {
public:
    Akk();
    ~Akk();

    void setInitialStateSpin(int doubledSpin);
    void setIntermediateStateSpin(int doubledSpin);
    void setFinalStateSpin(int doubledSpin);
    void setPopulatingGammaMixing(double delta, double deltaUncertainty = 0.0);
    void setDepopulatingGammaMixing(double delta, double deltaUncertainty = 0.0);

    double a22();
    double a22Uncertainty();
    double a24();
    double a24Uncertainty();
    double a42();
    double a42Uncertainty();
    double a44();
    double a44Uncertainty();

private:
    AkkPrivate *d;
};

#endif // AKK_H
