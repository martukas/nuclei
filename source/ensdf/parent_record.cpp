#include "parent_record.h"
#include "ensdf_types.h"

ParentRecord ParentRecord::parse(size_t& idx,
                                 const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !is(data[idx]))
    return ParentRecord();
  auto line = data[idx];

  ParentRecord ret;

  ret.nuclide = parse_nid(line.substr(0,5));
  ret.energy = parse_energy(line.substr(9,10), line.substr(19,2));
  ret.spin = parse_spin_parity(line.substr(21, 18));
  ret.hl = parse_halflife(line.substr(39, 16));
  ret.QP = parse_energy(line.substr(64,10), line.substr(74,2));
  ret.ionization = line.substr(76,4);

  return ret;
}

ParentRecord ParentRecord::from_ensdf(const std::string &line)
{
  ParentRecord prec;
  if (line.size() < 50)
    return prec;
  prec.nuclide = parse_nid(line.substr(0,5));
  prec.energy = parse_energy(line.substr(9,10), line.substr(19,2));
  prec.spin = parse_spin_parity(line.substr(21, 18));
  prec.hl = parse_halflife(line.substr(39, 16));
  return prec;
}

std::string ParentRecord::debug() const
{
  std::string ret;
  ret = nuclide.symbolicName()
      + " E=" + energy.to_string()
      + " SpinParity=" + spin.to_string()
      + " HL=" + hl.to_string()
      + " QP=" + QP.to_string()
      + " ion=" + ionization;
  return ret;
}
