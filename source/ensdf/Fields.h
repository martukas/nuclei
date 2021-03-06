#pragma once

#include <NucData/Uncert.h>
#include <NucData/Level.h>
#include <NucData/DecayInfo.h>
#include <NucData/nid.h>
#include <tuple>

bool is_uncertainty_id(const std::string& str);
bool has_uncertainty_id(const std::string& str);
std::string uncert_to_ensdf(Uncert::UncertaintyType t);
Uncert::UncertaintyType parse_uncert_type(const std::string& str);
std::string strip_uncert_type(const std::string& str);
Uncert parse_val_uncert(std::string val, std::string uncert);
Uncert parse_norm(std::string val, std::string uncert);
Energy parse_energy(std::string val, std::string uncert);

Uncert eval_mixing_ratio(Uncert vu, const std::string& mpol);

size_t occurrences(const std::string&s, const std::string&sub);
bool common_brackets(const std::string&s, char open, char close);
bool common_qualifiers(const std::string&);
DataQuality quality_of(const std::string&);
std::string strip_qualifiers(const std::string& s);
DataQuality scrape_quality(std::string& s);

Parity get_common_parity(std::string&s);

Spin parse_spin(const std::string& s);
Parity parse_parity(const std::string& s);
SpinParity parse_spin_parity(std::string data);
void simplify_logic(std::string& s);
std::pair<std::string, std::vector<std::string> > spin_split(const std::string& data);
SpinSet parse_spins(std::string data);

HalfLife parse_halflife(std::string record_orig);
std::string hl_to_ensdf(HalfLife hl);
DecayMode parse_decay_mode(std::string record);
DecayInfo parse_decay_info(std::string dsid);

std::string mode_to_ensdf(DecayMode);

std::string uncert_to_ensdf(Uncert);

NuclideId parse_nid(std::string nucid);
NuclideId parse_check_nid(std::string nucid);
std::string nid_to_ensdf(NuclideId, bool alt);
bool check_nid_parse(const std::string&, const NuclideId&);
