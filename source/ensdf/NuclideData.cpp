#include "NuclideData.h"
#include "Fields.h"

#include "custom_logger.h"
#include "qpx_util.h"

#include <boost/regex.hpp>

void NuclideData::merge_adopted(DecayData& decaydata,
                                double max_level_dif,
                                double max_gamma_dif) const
{
  for (LevelRecord& lev : decaydata.levels)
  {
    for (auto ad : adopted_levels)
    {
      std::list<LevelRecord> relevant_levels
          = ad.nearest_levels(lev.energy, decaydata.id.dsid,
                              max_level_dif);

      if (!relevant_levels.empty())
      {

        //    DBG << " B E F O R E\n"
        //        << "=======================================\n"
        //        << lev.debug() << "\n"
        //        << "=======================================\n"
        //           ;

        for (const LevelRecord& l
             : ad.nearest_levels(lev.energy,
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

void NuclideData::add_adopted(const AdoptedLevels &dec)
{
  adopted_levels.push_back(dec);
}

std::string NuclideData::add_decay(const DecayData& dec)
{
  auto base_name = dec.name();
  if (!dec.valid())
    base_name = "INVALID PARSE of " + dec.id.extended_dsid;

  // insert into decay map
  int count {0};
  auto disambiguated = base_name;
  while (decays.count(disambiguated))
  {
    count++;
    if (count > 1)
      disambiguated = base_name + " (alt."
          + boost::lexical_cast<std::string>(count)
          + ")";
    else
      disambiguated = base_name + " (alt.)";
  }

  //  if (!dec.valid())
  //    DBG << "<NuclideData> Adding decay: " << disambiguated;
  decays[disambiguated] = dec;

  return disambiguated;
}
