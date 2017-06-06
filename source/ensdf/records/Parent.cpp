#include "Parent.h"
#include "Fields.h"

bool ParentRecord::match(const std::string& line)
{
  return match_first(line, "\\sP");
}

ParentRecord::ParentRecord(ENSDFData& i)
{
  const auto& line = i.read();
  if (!match(line))
    return;

  nuclide = parse_nid(line.substr(0,5));
  energy = parse_energy(line.substr(9,10), line.substr(19,2));
  spins = parse_spins(line.substr(21, 18));
  hl = parse_halflife(line.substr(39, 16));
  QP = parse_energy(line.substr(64,10), line.substr(74,2));
  ionization = line.substr(76,4);
}

bool ParentRecord::valid() const
{
  return nuclide.valid();
}

std::string ParentRecord::debug() const
{
  std::string ret;
  ret = nuclide.symbolicName() + " PARENT "
      + " E=" + energy.to_string()
      + " SpinParity=" + spins.to_string()
      + " HL=" + hl.to_string()
      + " QP=" + QP.to_string()
      + " ion=" + ionization;
  return ret;
}
