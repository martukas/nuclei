#include "parent_record.h"
#include "ensdf_types.h"

bool ParentRecord::match(const std::string& line)
{
  return match_first(line, "\\sP");
}

ParentRecord::ParentRecord(size_t& idx,
                           const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !match(data[idx]))
    return;
  const auto& line = data[idx];

  nuclide = parse_nid(line.substr(0,5));
  energy = Energy(parse_val_uncert(line.substr(9,10), line.substr(19,2)));
  spin = parse_spin_parity(line.substr(21, 18));
  hl = parse_halflife(line.substr(39, 16));
  QP = Energy(parse_val_uncert(line.substr(64,10), line.substr(74,2)));
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
      + " SpinParity=" + spin.to_string()
      + " HL=" + hl.to_string()
      + " QP=" + QP.to_string()
      + " ion=" + ionization;
  return ret;
}
