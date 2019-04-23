#include <ensdf/NuclideData.h>
#include <ensdf/Fields.h>

#include <util/logger.h>
#include "qpx_util.h"

#include <boost/regex.hpp>

void NuclideData::merge_adopted(LevelsData &decaydata,
                                double max_level_dif,
                                double max_gamma_dif) const
{
  for (LevelRecord& lev : decaydata.levels)
  {
    for (auto ad : decays)
    {
      if (!ad.second.adopted)
        continue;

      std::list<LevelRecord> relevant_levels
          = ad.second.nearest_levels(lev.energy, decaydata.id.dsid,
                                     max_level_dif);

      if (!relevant_levels.empty())
      {

        //    DBG << " B E F O R E\n"
        //        << "=======================================\n"
        //        << lev.debug() << "\n"
        //        << "=======================================\n"
        //           ;

        for (const LevelRecord& l
             : ad.second.nearest_levels(lev.energy,
                                        decaydata.id.dsid,
                                        max_level_dif))
        {
          //      DBG << "+++++FoundE = " << l.debug();
          lev.merge_adopted(l, max_gamma_dif);
        }

        //    DBG << "                              A F T E R\n"
        //        << "---------------------------------------\n"
        //        << lev.debug() << "\n"
        //        << "---------------------------------------\n"
        //           ;
      }

      //    for (const LevelRecord& l : adopted_levels.levels)
      //      if (nearest_levels(l.energy, ).empty())
      //        gammas.push_back(g);
    }


  }
}

std::string NuclideData::add(const LevelsData& dec)
{
  auto base_name = dec.name();

  // insert into decay map
  int count {0};
  auto disambiguated = base_name;
  while (decays.count(disambiguated))
  {
    count++;
    if (count > 1)
      disambiguated = base_name + " (alt."
          + std::to_string(count)
          + ")";
    else
      disambiguated = base_name + " (alt.)";
  }

  //  if (!dec.valid())
  //    DBG << "<NuclideData> Adding decay: " << disambiguated;
  decays[disambiguated] = dec;

  return disambiguated;
}
