#include "ENSDFParser.h"
#include <cmath>
#include <iostream>

#include "XDecay.h"
#include "XNuclide.h"
#include "XEnergyLevel.h"
#include "XGammaTransition.h"
#include "custom_logger.h"
#include "qpx_util.h"

#include <list>
#include <utility>
#include <boost/regex.hpp>


ParentRecord ParentRecord::from_ensdf(const std::string &record)
{
  ParentRecord prec;
  if (record.size() < 50)
    return prec;
  prec.nuclide = NuclideId::from_ensdf(record.substr(0,5));
  prec.energy = Energy::from_nsdf(record.substr(9, 12));
  prec.hl = HalfLife::from_ensdf(record.substr(39, 16));
  prec.spin = SpinParity::from_ensdf(record.substr(21, 18));
  return prec;
}

std::string ParentRecord::to_string() const
{
  std::string ret;
  ret = nuclide.verboseName()
      + " " + energy.to_string()
      + " " + spin.to_string()
      + " " + hl.to_string();
  return ret;
}

BasicDecayData BasicDecayData::from_ensdf(const std::string &header, BlockIndices block)
{
  BasicDecayData decaydata;

  boost::regex decay_expr("^([\\sA-Z0-9]{5,5})\\s{4,4}[0-9]{1,3}[A-Z]{1,2}\\s+((?:B-|B\\+|EC|IT|A)\\sDECAY).*$");
  boost::smatch what;
  if (!boost::regex_match(header, what, decay_expr) || (what.size() < 2))
    return decaydata;

  decaydata.daughter = NuclideId::from_ensdf(what[1]);
  decaydata.decayType = parseDecayType(what[2]);
  decaydata.block = block;
  decaydata.dsid = boost::trim_copy(header.substr(9,30)); // saved for comparison with xref records
  return decaydata;
}

XDecay::Type BasicDecayData::parseDecayType(const std::string &tstring)
{
  if (tstring == "EC DECAY")
    return XDecay::ElectronCapture;
  if (tstring == "B+ DECAY")
    return XDecay::BetaPlus;
  if (tstring == "B- DECAY")
    return XDecay::BetaMinus;
  if (tstring == "IT DECAY")
    return XDecay::IsomericTransition;
  if (tstring == "A DECAY")
    return XDecay::Alpha;
  return XDecay::Undefined;
}

std::string BasicDecayData::to_string() const
{
  std::string ret;
  ret = daughter.verboseName() + " "
      + XDecay::DecayTypeAsText(decayType)
      + " block=" + std::to_string(block.first) + "-" + std::to_string(block.second)
      + " dsid=\"" + dsid + "\"";
  return ret;
}


std::list<uint16_t> ENSDFParser::aList;

ENSDFParser::ENSDFParser(uint16_t A)
  : a(A)
{
//  QSettings s;
//  // read data
//  QFile f(s.value("ensdfPath", ".").toString() + QString("/ensdf.%1").arg(a, int(3), int(10), QChar('0')));
//  f.open(QIODevice::ReadOnly | QIODevice::Text);
//  std::string c = QString::fromUtf8(f.readAll()).toStdString();

//  boost::split(contents, c, boost::is_any_of("\n"));

  parseBlocks();
}

const std::list<uint16_t> &ENSDFParser::aValues() // static
{
//  if (!aList.empty())
//    return aList;

//  QSettings s;
//  if (!s.contains("ensdfPath"))
//    return aList;

//  QDir dir(s.value("ensdfPath").toString());
//  if (!dir.exists())
//    return aList;

//  QStringList tmp(dir.entryList(QStringList("ensdf.???"), QDir::Files | QDir::Readable, QDir::Name));

//  for (int i=0; i<tmp.size(); i++) {
//    const QString aStr(tmp.at(i).right(3).remove(QRegExp("^0+")));
//    aList.push_back(aStr.toUInt());
//  }
  return aList;
}

uint16_t ENSDFParser::aValue() const
{
  return a;
}

const std::list<NuclideId> ENSDFParser::daughterNuclides() const
{
//  Q_ASSERT(m_decays.size() == m_decays.keys().toSet().size()); // check for duplicates
  std::list<NuclideId> ret;
  for (auto &n : m_decays)
    ret.push_back(n.first);
  return ret;
}

const std::list< std::pair<std::string, NuclideId> > ENSDFParser::decays(const NuclideId &daughterNuclide) const
{
//  Q_ASSERT(m_decays.value(daughterNuclide).size() == m_decays.value(daughterNuclide).keys().toSet().size()); // check for duplicates
  std::list< std::pair<std::string, NuclideId> > result;
  for (auto &i : m_decays.at(daughterNuclide))
    result.push_back(std::pair<std::string, NuclideId>(i.first, i.second.parents.at(0).nuclide));
  return result;
}

XDecay ENSDFParser::decay(const NuclideId &daughterNuclide, const std::string &decayName) const
{
  if (!m_adoptedlevels.count(daughterNuclide) ||
      !m_decays.count(daughterNuclide) ||
      !m_decays.at(daughterNuclide).count(decayName))
    return XDecay();

//  QSettings s;
//  double adoptedLevelMaxDifference = s.value("preferences/levelTolerance", 1.0).toDouble();
//  double gammaMaxDifference = s.value("preferences/gammaTolerance", 1.0).toDouble();

  double adoptedLevelMaxDifference = 1.0;
  double gammaMaxDifference = 1.0;

  std::map<Energy, XEnergyLevelPtr> levels;

  BlockIndices alpos = m_adoptedlevels.at(daughterNuclide);
  BasicDecayData decaydata = m_decays.at(daughterNuclide).at(decayName);
  double normalizeDecIntensToPercentParentDecay = 1.0;
  double normalizeGammaIntensToPercentParentDecay = 1.0;
  std::string dNucid = daughterNuclide.to_ensdf();

//  DBG << "parsing " << dNucid.toStdString();

  // process all adopted level sub-blocks
  XEnergyLevelPtr currentLevel(nullptr);
  // create index map for adopted levels
  std::map<Energy, StringSubList> adoptblocks;
  std::map<std::string, char> xrefs; // maps DSID to DSSYM (single letter)
  size_t laststart = -1;
  for (int i=alpos.first; i < alpos.second; i++) {
    const std::string line = contents.at(i);
    // extract cross reference records as long as first level was found (cross reference must be before 1st level)
    if (laststart == -1) {
      if (line.substr(0,8) == (dNucid + "  X"))
        xrefs[boost::trim_copy(line.substr(9, 30))] = line[8];
    }
    // find level records
    if (line.substr(0,9) == (dNucid + "  L ")) {
      if (laststart > 0) {
        size_t i1 = laststart;
        size_t i2 = i;
        StringSubList sl(i1, i2);
        if ((sl.first < sl.last) && (sl.last < contents.size()) && xrefs.count(decaydata.dsid))
          insertAdoptedLevelsBlock(&adoptblocks, sl, xrefs.at(decaydata.dsid));
      }
      laststart = i;
    }
  }
  if (laststart > 0) {
    StringSubList sl(laststart, alpos.second);
    if ((sl.first < sl.last) && (sl.last < contents.size()) && xrefs.count(decaydata.dsid))
      insertAdoptedLevelsBlock(&adoptblocks, sl, xrefs.at(decaydata.dsid));
  }

  // adopted levels block of current level in decay data set
  StringSubList currentadoptblock;

  // process decay block

//  DBG << "parsing size " << decaylines.size();

  for (int k=decaydata.block.first; k < decaydata.block.second; k++) {
    const std::string line = contents.at(k);

//    DBG << "c line " << line.toStdString();

    // process new gamma
    if ((levels.size() >= 8) && (line.substr(0,9) == (dNucid + "  G "))) {

//      DBG << "c1";

      Q_ASSERT(!levels.empty());

      // determine energy
      Energy e = Energy::from_nsdf(line.substr(9, 12));

      // determine intensity
      std::string instr = line.substr(21,8);
      boost::replace_all(instr, "(", "");
      boost::replace_all(instr, ")", "");
      boost::trim(instr);
      double  in = std::numeric_limits<double>::quiet_NaN();
      if (is_number(instr))
        in = boost::lexical_cast<double>(instr) * normalizeGammaIntensToPercentParentDecay;

      // determine multipolarity
      std::string mpol = boost::trim_copy(line.substr(31, 10));

      // determine delta
      UncertainDouble delta = parseEnsdfMixing(line.substr(41, 14), mpol);

      // parse adopted levels if necessary
      if ((delta.sign() != UncertainDouble::SignMagnitudeDefined) || mpol.empty()) {
        // create gamma map
        std::map<Energy, std::string> e2g;
        for (size_t i = currentadoptblock.first; i < currentadoptblock.last; ++i) {
          std::string ln = contents.at(i);
          if (ln.substr(0,9) ==  (dNucid + "  G ")) {
            Energy gk = Energy::from_nsdf(ln.substr(9, 12));
            if (gk.isValid())
              e2g[gk] = ln;
          }
        }
        // find gamma
        if (!e2g.empty()) {
          Energy foundE;
          const std::string gammastr = findNearest(e2g, e, &foundE);
          if (e-foundE < gammaMaxDifference/1000.0*e) {
            if (mpol.empty())
              mpol = boost::trim_copy(gammastr.substr(31, 10));

            if (delta.sign() != UncertainDouble::SignMagnitudeDefined) {
              UncertainDouble adptdelta = parseEnsdfMixing(gammastr.substr(41, 14), mpol);
              if (adptdelta.sign() > delta.sign())
                delta = adptdelta;
            }
          }
        }
      }

      // determine levels
      if (!levels.empty()) {
        XEnergyLevelPtr start = currentLevel;
        XEnergyLevelPtr destlvl = findNearest(levels, start->energy() - e);

        // gamma registers itself with the start and dest levels
        new XGammaTransition(e, in, mpol, delta, start, destlvl);
      }
    }
    // process new level
    else if (line.substr(0,9) == (dNucid + "  L ")) {

      currentLevel = XEnergyLevelPtr(new XEnergyLevel(XEnergyLevel::from_ensdf(line)));

      // get additional data from adopted leves record
      //   find closest entry
      if (!adoptblocks.empty()) {
        Energy foundE;
        currentadoptblock.clear();
        currentadoptblock = findNearest(adoptblocks, currentLevel->energy(), &foundE);
        Q_ASSERT(currentadoptblock.last >= currentadoptblock.first);
        if (qAbs((currentLevel->energy() - foundE).operator double()) > (adoptedLevelMaxDifference/1000.0*currentLevel->energy()))
          currentadoptblock.clear();
      }
      else {
        currentadoptblock.clear();
        Q_ASSERT(currentadoptblock.last >= currentadoptblock.first);
      }

      // if an appropriate entry was found, read its contents
      // set half life if necessary
      if (currentadoptblock.first != currentadoptblock.last) {
        std::string levelfirstline(contents.at(currentadoptblock.first));
        if (!currentLevel->halfLife().isValid()) {
          currentLevel->set_halflife(HalfLife::from_ensdf(levelfirstline.substr(39, 16)));
          if (!currentLevel->spin().valid())
            currentLevel->set_spin(SpinParity::from_ensdf(levelfirstline.substr(21, 18)));
        }
        // parse continuation records
        std::list<std::string> momentsRequestList;
        momentsRequestList.push_back("MOME2");
        momentsRequestList.push_back("MOMM1");
        std::vector<std::string> moms = extractContinuationRecords(currentadoptblock, momentsRequestList);
        currentLevel->set_q(Moment::from_ensdf(moms.at(0)));
        currentLevel->set_mu(Moment::from_ensdf(moms.at(1)));
//        DBG << levelfirstline << " === " << currentLevel->to_string();
      }

      levels[currentLevel->energy()] = currentLevel;
    }
    // process decay information
    else if (!levels.empty() && (line.substr(0,9) == (dNucid + "  E "))) {
      UncertainDouble ti = UncertainDouble::from_nsdf(line.substr(64, 10), line.substr(74, 2));
      if (ti.uncertaintyType() != UncertainDouble::UndefinedType) {
        ti.setSign(UncertainDouble::SignMagnitudeDefined);
      }
      else {
        UncertainDouble ib = UncertainDouble::from_nsdf(line.substr(21, 8), line.substr(29, 2));
        UncertainDouble ie = UncertainDouble::from_nsdf(line.substr(31, 8), line.substr(39, 2));
        ti = ib;
        if (ib.uncertaintyType() != UncertainDouble::UndefinedType && ie.uncertaintyType() != UncertainDouble::UndefinedType)
          ti += ie;
        else if (ie.uncertaintyType() != UncertainDouble::UndefinedType)
          ti = ie;
        else
          ti = UncertainDouble();
      }
      if (ti.uncertaintyType() != UncertainDouble::UndefinedType)
        currentLevel->setFeedIntensity(ti);
    }
    else if (!levels.empty() && (line.substr(0,9) == (dNucid + "  B "))) {
      UncertainDouble ib = UncertainDouble::from_nsdf(line.substr(21, 8), line.substr(29, 2));
      if (ib.hasFiniteValue()) {
        ib.setSign(UncertainDouble::SignMagnitudeDefined);
        ib *= normalizeDecIntensToPercentParentDecay;
        currentLevel->setFeedIntensity(ib);
      }
    }
    else if (!levels.empty() && (line.substr(0,9) == (dNucid + "  A "))) {
      UncertainDouble ia = UncertainDouble::from_nsdf(line.substr(21, 8), line.substr(29, 2));
      if (ia.hasFiniteValue()) {
        ia.setSign(UncertainDouble::SignMagnitudeDefined);
        ia *= normalizeDecIntensToPercentParentDecay;
        currentLevel->setFeedIntensity(ia);
      }
    }
    // process normalization records
    else if (line.substr(0,9) == (dNucid + "  N ")) {
      double br = norm(line.substr(31, 8), 1.0);
      double nb = norm(line.substr(41, 8), 1.0);
      normalizeDecIntensToPercentParentDecay = nb * br;
      double nr = norm(line.substr(9, 10), 1.0);
      normalizeGammaIntensToPercentParentDecay = nr * br;
    }
    else if (line.substr(0,9) == (dNucid + " PN ")) {
      normalizeDecIntensToPercentParentDecay = norm(line.substr(41, 8), normalizeDecIntensToPercentParentDecay);
      normalizeGammaIntensToPercentParentDecay = norm(line.substr(9, 10), normalizeGammaIntensToPercentParentDecay);
    }
  }

  // create relevant parent levels and collect parent half-lifes
  std::map<Energy, XEnergyLevelPtr> plevels;
  std::vector<HalfLife> pHl;
  foreach (ParentRecord p, decaydata.parents) {
    pHl.push_back(p.hl);

    XEnergyLevelPtr plv(new XEnergyLevel(p.energy, p.spin, p.hl));
    plv->setFeedingLevel(true);
    plevels[p.energy] = plv;
  }

  if (!plevels.empty() && (plevels.begin()->second->energy() > 0.0))
  {
      XEnergyLevelPtr plv(new XEnergyLevel(Energy(0.0, UncertainDouble::SignMagnitudeDefined), SpinParity(), HalfLife()));
      plv->setFeedingLevel(false);
      plevels[plv->energy()] = plv;
  }

  XNuclidePtr parent_nuclide(new XNuclide(decaydata.parents.at(0).nuclide, pHl));
  parent_nuclide->addLevels(plevels);
  XNuclidePtr daughter_nuclide(new XNuclide(decaydata.daughter));
  daughter_nuclide->addLevels(levels);

  return XDecay(decayName, parent_nuclide, daughter_nuclide, decaydata.decayType);
}

double ENSDFParser::norm(std::string rec, double def_value)
{
  boost::replace_all(rec, "(", "");
  boost::replace_all(rec, ")", "");
  boost::trim(rec);
  if (is_number(rec))
    def_value = boost::lexical_cast<double>(rec);
  return def_value;
}



UncertainDouble ENSDFParser::parseEnsdfMixing(const std::string &mstr, const std::string &multipolarity)
{
  Q_ASSERT(mstr.size() == 14);

  // special case for pure multipolarities
  if (boost::trim_copy(mstr).empty()) {
    std::string tmp(multipolarity);
    boost::replace_all(tmp, "(", "");
    boost::replace_all(tmp, ")", "");
    if (tmp.size() == 2)
      return UncertainDouble(0.0, 1, UncertainDouble::SignMagnitudeDefined);
    else
      return UncertainDouble();
  }

  // mixed case
//  return parseUncertainty(mstr.left(8).trimmed(), mstr.right(6).trimmed().replace(' ', ""));
  return UncertainDouble::from_nsdf(mstr.substr(0,8), mstr.substr(8,6));

}

/**
 * @brief ENSDFParser::insertAdoptedLevelsBlock
 * @param adoptblocks map of adopted levels (target)
 * @param newblock block to add
 * @param dssym Symbol of the current decay. Used to filter levels and adjust energies according to XREF records
 */
void ENSDFParser::insertAdoptedLevelsBlock(std::map<Energy, StringSubList> *adoptblocks, const StringSubList &newblock, char dssym) const
{
  Q_ASSERT(adoptblocks);
  // get xref record

  std::list<std::string> req;
  req.push_back("XREF");
  std::vector<std::string> xreflist = extractContinuationRecords(newblock, req);
  std::string xref = xreflist.at(0);

  // filter data sets

  // -(AB) case (do not add level if dssym is contained in the parentheses)
  if ((xref.substr(0,2) == "-(")
      && (xref[xref.size()-1] == ')')
      && boost::contains(xref, std::string(1,dssym)))
    return;

  // exit if xref is neither "+" (level valid for all datasets) nor -(...) not containing dssymb
  // nor contains dssym
  if (xref != "+"
      && !(xref.substr(0,20) == "-(")
      && !boost::contains(xref,std::string(1,dssym)))
    return;

  // if this point is reached the level will be added in any case

  // read energy from level record
  Energy e = Energy::from_nsdf(contents.at(newblock.first).substr(9, 12));


  // for the A(E1) case the energy must be modified
  QRegExp er(QString(dssym) + "\\(([.\\d]+)\\)");
  QString xs = QString::fromStdString(xref);
  if (er.indexIn(xs) >= 0) {
    Energy matchedE = Energy::from_nsdf(er.cap(1).toStdString());
    DBG << "ENERGY FROM REGEXP " << er.cap(1).toStdString() << " --> " << matchedE.to_string() << " valid " << matchedE.isValid();

    //std::cerr << "Current xref: " << xref.toStdString() << " current dssymb: " << dssym << std::endl;
    //std::cerr << "Energy translation, old: " << e << " new: " << matchedE << std::endl;
    if (matchedE.isValid())
      e = matchedE;
  }

  Q_ASSERT(newblock.last >= newblock.first);

  // add record
  (*adoptblocks)[e] = newblock;
}

/**
 * @brief ENSDFParser::extractContinuationRecords
 * @param adoptedblock block to search continuation records in
 * @param requestedRecords list of requested records
 * @param typeOfContinuedRecord type of record (default: L(evel))
 * @return list of found records (same size as requestedRecords, empty strings if no record was found)
 */
std::vector<std::string> ENSDFParser::extractContinuationRecords(const StringSubList &adoptedblock,
                                                                 const std::list<std::string> &requestedRecords,
                                                                 char typeOfContinuedRecord) const
{
  // fetch records
  boost::regex crecre("^[A-Z0-9\\s]{5,5}[A-RT-Z0-9] " + std::string(1,typeOfContinuedRecord) + " (.*)$");
  std::vector<std::string> crecs;
  for (size_t i = adoptedblock.first; i < adoptedblock.last; ++i) {
    boost::smatch what;
    if (boost::regex_search(contents.at(i), what, crecre) && (what.size() > 1))
      crecs.push_back(what[1]);
  }
  QStringList crecs2;
  // remove record id from beginning of string
  for (int i=0; i<crecs.size(); i++) {
    crecs[i].erase(0, 9);
    crecs2.append(QString::fromStdString(crecs.at(i)));
  }
  // join lines and then split records
  QString tmp(crecs2.join("$"));
  crecs2 = tmp.split('$');
  for (int i=0; i<crecs2.size(); i++)
    crecs2[i] = crecs2[i].trimmed();
  // search and parse requested fields
  std::vector<std::string> result;
  for ( auto &req : requestedRecords) {
    QString rstr;
    for (int i=0; i<crecs2.size(); i++) {
      if (crecs2.at(i).startsWith(QString::fromStdString(req))) {
        rstr = crecs2.at(i).mid(5).trimmed();
        break;
      }
    }
    if (rstr.startsWith('='))
      rstr.remove(0, 1);
    result.push_back(rstr.toStdString());
  }
  return result;
}

template <typename T>
T ENSDFParser::findNearest(const std::map<Energy, T> &map, const Energy &val, Energy *foundVal) const
{
  Q_ASSERT(!map.empty());

  typename std::map<Energy, T>::const_iterator low, prev;
  low = map.lower_bound(val);
  if (low == map.end())
    low--;
   else if (low != map.begin()) {
      prev = low;
      --prev;
      if ((val - prev->first) < (low->first - val))
        low = prev;
  }

  if (foundVal)
    (*foundVal) = low->first;

  return low->second;
}

void ENSDFParser::parseBlocks()
{
  // create list of block boundaries
  // end index points behind last line of block!
  std::list<BlockIndices> bb;
  size_t from = 0;
  boost::regex emptyline("^\\s*$");
  for (size_t i=0; i < contents.size(); ++i) {
    if (boost::regex_match(contents.at(i), emptyline)) {
      if (i-from > 1)
        bb.push_back(BlockIndices(from, i));
      from = i + 1;
    }
  }
  if (from < contents.size())
    bb.push_back(BlockIndices(from, contents.size()-1));

  // prepare regexps
  boost::regex adopted_levels_expr("^([\\sA-Z0-9]{5,5})    ADOPTED LEVELS.*$");
  boost::regex decay_expr("^([\\sA-Z0-9]{5,5})\\s{4,4}[0-9]{1,3}[A-Z]{1,2}\\s+((?:B-|B\\+|EC|IT|A)\\sDECAY).*$");

  // recognize blocks
  foreach (const BlockIndices &block, bb) {
    std::string header = contents.at(block.first);
//    DBG << "examining block [" << block.first << "-" << block.second << "] " << header;

    // adopted levels
    boost::smatch what;
    if (boost::regex_match(header, what, adopted_levels_expr) && (what.size() > 1)) {
      m_adoptedlevels[NuclideId::from_ensdf(what[1])] = block;
//      DBG << "Adopted levels   " << NuclideId::from_ensdf(what[1]).verboseName()
//          << " block=" << block.first << "-" << block.second;
//      for (size_t i=block.first; i < block.second; ++i)
//        DBG << "             AL  "<< contents.at(i);
    }

    // decays
    else if (boost::regex_match(header, what, decay_expr) && (what.size() > 1)) {
      BasicDecayData decaydata = BasicDecayData::from_ensdf(header, block);

//      DBG << "Basic decay data " << decaydata.to_string();

      boost::regex filter("^[\\s0-9A-Z]{5,5}\\s\\sP[\\s0-9].*$");
      for (size_t i=block.first; i < block.second; ++i) {
        if (boost::regex_match(contents.at(i), filter)) {
          decaydata.parents.push_back(ParentRecord::from_ensdf(contents.at(i)));
//          DBG << "  Parent         " << decaydata.parents.back().to_string();
        }
//        else
//          DBG << "             DEC " << contents.at(i);
      }

      if (decaydata.parents.empty()) {
        DBG <<   " BROKEN RECORD FOR " << decaydata.to_string();
        // broken records. skipping
        continue;
      }

      // create decay string
      //   get reference to daughter map to work with (create and insert if necessary)
      std::map<std::string, BasicDecayData> &decmap = m_decays[decaydata.daughter];

      std::vector<std::string> hlstrings;
      foreach (const ParentRecord &prec, decaydata.parents) {
        Q_ASSERT(prec.nuclide == decaydata.parents.at(0).nuclide); // check "same parent/different half-life" scheme
        hlstrings.push_back(prec.hl.to_string());
      }

      const ParentRecord &prec(decaydata.parents.at(0));
      std::string decayname  = prec.nuclide.symbolicName();
      if (prec.energy > 0.0)
        decayname += "m";
      decayname += " → " + XDecay::DecayTypeAsText(decaydata.decayType);
      if (!hlstrings.empty())
        decayname += ", " + join(hlstrings, " + ");

      // insert into decay map
      while (decmap.count(decayname))
        decayname += " (alt.)";
      decmap[decayname] = decaydata;
    }
    else
    {
//      DBG << "Unprocessed block begin " << block.first;
//      for (size_t i=block.first; i < block.second; ++i)
//        DBG << contents.at(i);
//      DBG << "Unprocessed block end " << block.second;
    }

  }
}

