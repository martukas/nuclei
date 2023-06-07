//
//  Copyright 2012 CERN, João Guilherme Martins Correia & Marcelo Baptista Barbosa.
//  All rights reserved.
//

#include "Akk.h"
#include <cmath>
#include <gsl/gsl_sf_coupling.h>

class AkkPrivate {

public:
    bool valid;

    // Input variables
    int dbl_ji; // initial state spin
    int dbl_j; // intermediate state spin
    int dbl_jf; // final state spin
    double d1;
    double d1Uncert;
    double d2;
    double d2Uncert;
    double firstAl;
    double firstAl1;
    double firstB211;
    double firstB212;
    double firstB222;
    double firstB411;
    double firstB412;
    double firstB422;
    double secondAl;
    double secondAl1;
    double secondB211;
    double secondB212;
    double secondB222;
    double secondB411;
    double secondB412;
    double secondB422;
    // Output variables
    double a22;
    double a22Uncert;
    double a24;
    double a24Uncert;
    double a42;
    double a42Uncert;
    double a44;
    double a44Uncert;

    AkkPrivate()
        : valid(false),
          dbl_ji(0),
          dbl_j(0),
          dbl_jf(0),
          d1(0.0),
          d1Uncert(0.0),
          d2(0.0),
          d2Uncert(0.0),
          firstAl(1.0),
          firstAl1(1.0),
          firstB211(1.0),
          firstB212(1.0),
          firstB222(1.0),
          firstB411(1.0),
          firstB412(1.0),
          firstB422(1.0),
          secondAl(1.0),
          secondAl1(1.0),
          secondB211(1.0),
          secondB212(1.0),
          secondB222(1.0),
          secondB411(1.0),
          secondB412(1.0),
          secondB422(1.0),
          a22(0.0),
          a22Uncert(0.0),
          a24(0.0),
          a24Uncert(0.0),
          a42(0.0),
          a42Uncert(0.0),
          a44(0.0),
          a44Uncert(0.0)
    {
    }

    void calculateAkk()
    {
        if (valid)
            return;

        //  A2 (gamma 1)
        double a2gamma1Uncert = 0.0;
        double a2gamma1 = calculateComponent(2, dbl_ji, dbl_j, firstAl1, firstAl, firstB211, firstB212, firstB222, d1, d1Uncert, &a2gamma1Uncert);

        //  A4 (gamma 1)
        double a4gamma1Uncert = 0.0;
        double a4gamma1 = calculateComponent(4, dbl_ji, dbl_j, firstAl1, firstAl, firstB411, firstB412, firstB422, d1, d1Uncert, &a4gamma1Uncert);

        // using negative delta value for second gamma value according to Krane/Steffen 1970
        //  A2 (gamma 2)
        double a2gamma2Uncert = 0.0;
        double a2gamma2 = calculateComponent(2, dbl_jf, dbl_j, secondAl1, secondAl, secondB211, secondB212, secondB222, -d2, d2Uncert, &a2gamma2Uncert);

        //  A4 (gamma 2)
        double a4gamma2Uncert = 0.0;
        double a4gamma2 = calculateComponent(4, dbl_jf, dbl_j, secondAl1, secondAl, secondB411, secondB412, secondB422, -d2, d2Uncert, &a4gamma2Uncert);

        a22 = a2gamma1*a2gamma2;
        a22Uncert = std::sqrt(a2gamma1*a2gamma1 * a2gamma2Uncert*a2gamma2Uncert + a2gamma2*a2gamma2 * a2gamma1Uncert*a2gamma1Uncert);
        a24 = a2gamma1*a4gamma2;
        a24Uncert = std::sqrt(a2gamma1*a2gamma1 * a4gamma2Uncert*a4gamma2Uncert + a4gamma2*a4gamma2 * a2gamma1Uncert*a2gamma1Uncert);
        a42 = a4gamma1*a2gamma2;
        a42Uncert = std::sqrt(a4gamma1*a4gamma1 * a2gamma2Uncert*a2gamma2Uncert + a2gamma2*a2gamma2 * a4gamma1Uncert*a4gamma1Uncert);
        a44 = a4gamma1*a4gamma2;
        a44Uncert = std::sqrt(a4gamma1*a4gamma1 * a4gamma2Uncert*a4gamma2Uncert + a4gamma2*a4gamma2 * a4gamma1Uncert*a4gamma1Uncert);

        valid = true;
    }

    double calculateComponent(int k, int jx_2, int j_2, double Al1, double Al, double Bx11, double Bx12, double Bx22,
                              double delta, double deltaUncertainty, double * resultUncertainty)
    {
        int lx_2;
        if (jx_2 >= j_2)
            lx_2 = jx_2-j_2;
        else
            lx_2 = j_2-jx_2;

        const int lxPlusOne_2 = lx_2 + 2;

        const double de = sqrt(Al1/Al)*delta;
        const double deUc = sqrt(Al1/Al)*deltaUncertainty;

        const int sign = ((jx_2+j_2)/2+1)%2 ? -1 : 1; // equivalent to pow(-1., jx_2/2.+j_2/2.+1.)

        const double f11 = Bx11*sign*sqrt((lx_2+1.)*(lx_2+1.)*(2*k+1.)*(j_2+1.))
        *gsl_sf_coupling_3j(lx_2, lx_2, 2*k, 2, -2, 0.)
        *gsl_sf_coupling_6j(j_2, j_2, 2*k, lx_2, lx_2, jx_2);

        const double f12 = Bx12*sign*sqrt((lx_2+1.)*(lxPlusOne_2+1.)*(2*k+1.)*(j_2+1.))
        *gsl_sf_coupling_3j(lx_2, lxPlusOne_2, 2*k, 2, -2, 0.)
        *gsl_sf_coupling_6j(j_2, j_2, 2*k, lx_2, lxPlusOne_2, jx_2);

        const double f22 = Bx22*sign*sqrt((lxPlusOne_2+1.)*(lxPlusOne_2+1.)*(2*k+1.)*(j_2+1.))
        *gsl_sf_coupling_3j(lxPlusOne_2, lxPlusOne_2, 2*k, 2, -2, 0.)
        *gsl_sf_coupling_6j(j_2, j_2, 2*k, lxPlusOne_2, lxPlusOne_2, jx_2);

        // propagate uncertainty
        // the partial derivative of the returned function r(de) is: d r(de) / d(de) = (2 (-f12 + de (-f11 + de f12 + f22)))/(1 + de^2)^2
        *resultUncertainty = ( 2.*(-f12+de*(-f11+de*f12+f22)) / (1+de*de) / (1+de*de) ) * deUc;

        return (f11-2.*de*f12+de*de*f22)/(1+de*de);
    }

};


Akk::Akk()
    : d(new AkkPrivate)
{
}

Akk::~Akk()
{
    delete d;
}

void Akk::setInitialStateSpin(int doubledSpin)
{
    d->dbl_ji = doubledSpin;
    d->valid = false;
}

void Akk::setIntermediateStateSpin(int doubledSpin)
{
    d->dbl_j = doubledSpin;
    d->valid = false;
}

void Akk::setFinalStateSpin(int doubledSpin)
{
    d->dbl_jf = doubledSpin;
    d->valid = false;
}

void Akk::setPopulatingGammaMixing(double delta, double deltaUncertainty)
{
    d->d1 = delta;
    d->d1Uncert = deltaUncertainty;
    d->valid = false;
}

void Akk::setDepopulatingGammaMixing(double delta, double deltaUncertainty)
{
    d->d2 = delta;
    d->d2Uncert = deltaUncertainty;
    d->valid = false;
}

double Akk::a22()
{
    d->calculateAkk();
    return d->a22;
}

double Akk::a22Uncertainty()
{
    d->calculateAkk();
    return d->a22Uncert;
}

double Akk::a24()
{
    d->calculateAkk();
    return d->a24;
}

double Akk::a24Uncertainty()
{
    d->calculateAkk();
    return d->a24Uncert;
}

double Akk::a42()
{
    d->calculateAkk();
    return d->a42;
}

double Akk::a42Uncertainty()
{
    d->calculateAkk();
    return d->a42Uncert;
}

double Akk::a44()
{
    d->calculateAkk();
    return d->a44;
}

double Akk::a44Uncertainty()
{
    d->calculateAkk();
    return d->a44Uncert;
}
