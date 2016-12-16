#ifndef ENSDF_TYPES_H
#define ENSDF_TYPES_H

#include "UncertainDouble.h"
#include "Level.h"
#include "nid.h"

bool is_uncertainty_id(const std::string& str);
UncertainDouble as_uncertain(std::string val, std::string uncert);

DataQuality quality_of(const std::string&);
std::string strip_qualifiers(const std::string& original);

Spin as_spin(const std::string& s);
Parity as_parity(const std::string& s);
SpinParity as_spin_parity(std::string data);

NuclideId as_nucid(const std::string& nucid);
Moment as_moment(const std::string& s);
Energy as_energy(const std::string& record);
HalfLife as_halflife(const std::string& record);
Level  as_level(const std::string& record);



#endif
