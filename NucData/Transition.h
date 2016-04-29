#ifndef Transition_H
#define Transition_H

#include <stdint.h>
#include <memory>
#include "Energy.h"
#include "UncertainDouble.h"

class Level;

class Transition
{
public:
    Transition(Energy energy, double intensity,
                    const std::string &multipol, UncertainDouble delta,
                    std::shared_ptr<Level> start, std::shared_ptr<Level> dest);

    virtual ~Transition();

    Energy energy() const;
    double intensity() const;
    std::string multipolarity() const;
    const UncertainDouble &delta() const;

    std::string intensityAsText() const;
    std::string multipolarityAsText() const;

    std::shared_ptr<Level> depopulatedLevel() const;
    std::shared_ptr<Level> populatedLevel() const;

    double widthFromOrigin() const;

private:
    static double gauss(const double x, const double sigma);

    Energy energy_;
    double intensity_;
    std::string m_mpol;
    UncertainDouble m_delta;
    std::shared_ptr<Level> from_, to_;

};

typedef std::shared_ptr<Transition> TransitionPtr;

#endif // Transition_H
