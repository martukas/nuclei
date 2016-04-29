#ifndef Level_H
#define Level_H

#include <stdint.h>
#include <limits>
#include <memory>
#include <list>
#include "HalfLife.h"
#include "Energy.h"
#include "SpinParity.h"
#include "Moment.h"

class Transition;


class Level
{
public:

  Level()
    : isonum(0)
    , feedinglevel(false)
  {}

    Level(Energy energy, SpinParity spin,
                HalfLife halfLife = HalfLife(std::numeric_limits<double>::infinity()),
                unsigned int isomerNum = 0);

    static Level from_ensdf(std::string record);

    Energy energy() const;
    SpinParity spin() const;
    HalfLife halfLife() const;
    unsigned int isomerNum() const;
    UncertainDouble normalizedFeedIntensity() const;
    Moment mu() const;
    Moment q() const;

    void set_mu(const Moment &);
    void set_q(const Moment &);
    void set_halflife(const HalfLife&);
    void set_spin(const SpinParity&);

    const std::list<std::shared_ptr<Transition>> &depopulatingTransitions() const;
    const std::list<std::shared_ptr<Transition>> &populatingTransitions() const;

    std::string to_string() const;

    void setFeedIntensity(UncertainDouble intensity);
    void setFeedingLevel(bool isfeeding);

    bool isFeedingLevel() const;

    friend class Transition;

private:
    Energy energy_;
    SpinParity spin_;
    HalfLife halflife_;
    Moment m_Q, m_mu; // quadrupole and magnetic moments
    unsigned int isonum; // >0 for isomeric levels (counted from low energy to high), 0 otherwise


    UncertainDouble feedintens; // says how often this level is directly fed per 100 parent decays
    bool feedinglevel; // true if this is belonging to a parent nuclide and is a starting point for decays


    mutable std::list<std::shared_ptr<Transition>> m_populatingTransitions;
    mutable std::list<std::shared_ptr<Transition>> m_depopulatingTransitions;
};

typedef std::shared_ptr<Level> LevelPtr;

#endif // Level_H
