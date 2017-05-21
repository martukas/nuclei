#pragma once

#include "UncertainDouble.h"
#include "Level.h"
#include "DecayMode.h"
#include "nid.h"

bool is_uncertainty_id(const std::string& str);
UncertainDouble parse_val_uncert(std::string val, std::string uncert);
UncertainDouble parse_norm(std::string val, std::string uncert);
Energy parse_energy(std::string val, std::string uncert);

UncertainDouble eval_mixing_ratio(UncertainDouble vu, const std::string& mpol);

DataQuality quality_of(const std::string&);
std::string strip_qualifiers(const std::string& original);

Spin parse_spin(const std::string& s);
Parity parse_parity(const std::string& s);
SpinParity parse_spin_parity(std::string data);

Moment parse_moment(const std::string& s);
HalfLife parse_halflife(const std::string& record);

DecayMode parse_decay_mode(std::string record);

std::string mode_to_ensdf(DecayMode);

NuclideId parse_nid(std::string nucid);
NuclideId parse_check_nid(std::string nucid);
std::string nid_to_ensdf(NuclideId, bool alt);
bool check_nid_parse(const std::string&, const NuclideId&);
