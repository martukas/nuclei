#include "ENSDFParser.h"
#include <QDir>
#include <QFile>
#include <QChar>
#include <QMap>
#include <QLocale>
#include <QSettings>
#include <QSet>
#include <cmath>
#include <iostream>

#include "Decay.h"
#include "Nuclide.h"
#include "EnergyLevel.h"
#include "GammaTransition.h"
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

Decay::Type BasicDecayData::parseDecayType(const std::string &tstring)
{
  if (tstring == "EC DECAY")
    return Decay::ElectronCapture;
  if (tstring == "B+ DECAY")
    return Decay::BetaPlus;
  if (tstring == "B- DECAY")
    return Decay::BetaMinus;
  if (tstring == "IT DECAY")
    return Decay::IsomericTransition;
  if (tstring == "A DECAY")
    return Decay::Alpha;
  return Decay::Undefined;
}

std::string BasicDecayData::to_string() const
{
  std::string ret;
  ret = daughter.verboseName() + " "
      + Decay::decayTypeAsText(decayType).toStdString()
      + " block=" + std::to_string(block.first) + "-" + std::to_string(block.second)
      + " dsid=\"" + dsid + "\"";
  return ret;
}


QList<unsigned int> ENSDFParser::aList;

ENSDFParser::ENSDFParser(unsigned int A)
  : a(A)
{
  QSettings s;
  // read data
  QFile f(s.value("ensdfPath", ".").toString() + QString("/ensdf.%1").arg(a, int(3), int(10), QChar('0')));
  f.open(QIODevice::ReadOnly | QIODevice::Text);
  std::string c = QString::fromUtf8(f.readAll()).toStdString();

  boost::split(contents, c, boost::is_any_of("\n"));

  parseBlocks();
}

const QList<unsigned int> &ENSDFParser::aValues() // static
{
  if (!aList.isEmpty())
    return aList;

  QSettings s;
  if (!s.contains("ensdfPath"))
    return aList;

  QDir dir(s.value("ensdfPath").toString());
  if (!dir.exists())
    return aList;

  QStringList tmp(dir.entryList(QStringList("ensdf.???"), QDir::Files | QDir::Readable, QDir::Name));

  for (int i=0; i<tmp.size(); i++) {
    const QString aStr(tmp.at(i).right(3).remove(QRegExp("^0+")));
    aList.append(aStr.toUInt());
  }
  return aList;
}

unsigned int ENSDFParser::aValue() const
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

const std::list< std::pair<QString, NuclideId> > ENSDFParser::decays(const NuclideId &daughterNuclide) const
{
//  Q_ASSERT(m_decays.value(daughterNuclide).size() == m_decays.value(daughterNuclide).keys().toSet().size()); // check for duplicates
  std::list< std::pair<QString, NuclideId> > result;
  for (auto &i : m_decays.at(daughterNuclide))
    result.push_back(std::pair<QString, NuclideId>(QString::fromStdString(i.first), i.second.parents.at(0).nuclide));
  return result;
}

QSharedPointer<Decay> ENSDFParser::decay(const NuclideId &daughterNuclide, const std::string &decayName) const
{
  if (!m_adoptedlevels.count(daughterNuclide) ||
      !m_decays.count(daughterNuclide) ||
      !m_decays.at(daughterNuclide).count(decayName))
    return QSharedPointer<Decay>();

  QSettings s;
  double adoptedLevelMaxDifference = s.value("preferences/levelTolerance", 1.0).toDouble();
  double gammaMaxDifference = s.value("preferences/gammaTolerance", 1.0).toDouble();


  QMap<Energy, EnergyLevel*> levels;

  BlockIndices alpos = m_adoptedlevels.at(daughterNuclide);
  BasicDecayData decaydata = m_decays.at(daughterNuclide).at(decayName);
  double normalizeDecIntensToPercentParentDecay = 1.0;
  double normalizeGammaIntensToPercentParentDecay = 1.0;
  QString dNucid = QString::fromStdString(daughterNuclide.to_ensdf());

//  DBG << "parsing " << dNucid.toStdString();

  // process all adopted level sub-blocks
  EnergyLevel *currentLevel = 0;
  QLocale clocale("C");
  bool convok;
  // create index map for adopted levels
  QMap<Energy, StringSubList> adoptblocks;
  std::map<std::string, char> xrefs; // maps DSID to DSSYM (single letter)
  size_t laststart = -1;
  for (int i=alpos.first; i < alpos.second; i++) {
    const QString line = QString::fromStdString(contents.at(i));
    // extract cross reference records as long as first level was found (cross reference must be before 1st level)
    if (laststart == -1) {
      if (line.startsWith(dNucid + "  X"))
        xrefs[line.mid(9, 30).trimmed().toStdString()] = line.at(8).toLatin1();
    }
    // find level records
    if (line.startsWith(dNucid + "  L ")) {
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
    const QString line = QString::fromStdString(contents.at(k));

//    DBG << "c line " << line.toStdString();

    // process new gamma
    if ((levels.size() > 1) && line.startsWith(dNucid + "  G ")) {

//      DBG << "c1";

      Q_ASSERT(!levels.isEmpty());

      // determine energy
      Energy e = Energy::from_nsdf(line.mid(9, 12).toStdString());

      // determine intensity
      QString instr(line.mid(21,8));
      instr.remove('(').remove(')');
      double in = clocale.toDouble(instr, &convok);
      if (!convok)
        in = std::numeric_limits<double>::quiet_NaN();
      else
        in *= normalizeGammaIntensToPercentParentDecay;

      // determine multipolarity
      QString mpol(line.mid(31, 10).trimmed());

      // determine delta
      UncertainDouble delta = parseEnsdfMixing(line.mid(41, 14), mpol);

      // parse adopted levels if necessary
      if ((delta.sign() != UncertainDouble::SignMagnitudeDefined) || mpol.isEmpty()) {
        // create gamma map
        QMap<Energy, QString> e2g;
        for (size_t i = currentadoptblock.first; i < currentadoptblock.last; ++i) {
          QString ln = QString::fromStdString(contents.at(i));
          if (ln.startsWith(dNucid + "  G ")) {
            Energy gk = Energy::from_nsdf(ln.mid(9, 12).toStdString());
            if (gk.isValid())
              e2g.insert(gk, ln);
          }
        }
        // find gamma
        if (!e2g.isEmpty()) {
          Energy foundE;
          const QString gammastr = findNearest(e2g, e, &foundE);
          if (e-foundE < gammaMaxDifference/1000.0*e) {
            if (mpol.isEmpty())
              mpol = gammastr.mid(31, 10).trimmed();

            if (delta.sign() != UncertainDouble::SignMagnitudeDefined) {
              UncertainDouble adptdelta = parseEnsdfMixing(gammastr.mid(41, 14), mpol);
              if (adptdelta.sign() > delta.sign())
                delta = adptdelta;
            }
          }
        }
      }

      // determine levels
      if (!levels.isEmpty()) {
        EnergyLevel *start = currentLevel;
        EnergyLevel *destlvl = findNearest(levels, start->energy() - e);

        // gamma registers itself with the start and dest levels
        new GammaTransition(e, in, mpol, delta, start, destlvl);
      }
    }
    // process new level
    else if (line.startsWith(dNucid + "  L ")) {

      currentLevel = new EnergyLevel(EnergyLevel::from_ensdf(line.toStdString()));

      // get additional data from adopted leves record
      //   find closest entry
      if (!adoptblocks.isEmpty()) {
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

      levels.insert(currentLevel->energy(), currentLevel);
    }
    // process decay information
    else if (!levels.isEmpty() && line.startsWith(dNucid + "  E ")) {
//      UncertainDouble ti = parseUncertainty(line.mid(64, 10).remove("(").remove(")"), line.mid(74, 2));
      UncertainDouble ti = UncertainDouble::from_nsdf(line.mid(64, 10).remove("(").remove(")").toStdString(),
                                        line.mid(74, 2).toStdString());
      if (ti.uncertaintyType() != UncertainDouble::UndefinedType) {
        ti.setSign(UncertainDouble::SignMagnitudeDefined);
      }
      else {
//        UncertainDouble ib = parseUncertainty(line.mid(21, 8).remove("(").remove(")"), line.mid(29, 2));
//        UncertainDouble ie = parseUncertainty(line.mid(31, 8).remove("(").remove(")"), line.mid(39, 2));
        UncertainDouble ib = UncertainDouble::from_nsdf(line.mid(21, 8).remove("(").remove(")").toStdString(),
                                                        line.mid(29, 2).toStdString());
        UncertainDouble ie = UncertainDouble::from_nsdf(line.mid(31, 8).remove("(").remove(")").toStdString(),
                                                        line.mid(39, 2).toStdString());
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
    else if (!levels.isEmpty() && line.startsWith(dNucid + "  B ")) {
//      UncertainDouble ib = parseUncertainty(line.mid(21, 8).remove("(").remove(")"), line.mid(29, 2));
      UncertainDouble ib = UncertainDouble::from_nsdf(line.mid(21, 8).remove("(").remove(")").toStdString(),
                                                      line.mid(29, 2).toStdString());
      if (ib.hasFiniteValue()) {
        ib.setSign(UncertainDouble::SignMagnitudeDefined);
        ib *= normalizeDecIntensToPercentParentDecay;
        currentLevel->setFeedIntensity(ib);
      }
    }
    else if (!levels.isEmpty() && line.startsWith(dNucid + "  A ")) {
//      UncertainDouble ia = parseUncertainty(line.mid(21, 8).remove("(").remove(")"), line.mid(29, 2));
      UncertainDouble ia = UncertainDouble::from_nsdf(line.mid(21, 8).remove("(").remove(")").toStdString(),
                                                      line.mid(29, 2).toStdString());
      if (ia.hasFiniteValue()) {
        ia.setSign(UncertainDouble::SignMagnitudeDefined);
        ia *= normalizeDecIntensToPercentParentDecay;
        currentLevel->setFeedIntensity(ia);
      }
    }
    // process normalization records
    else if (line.startsWith(dNucid + "  N ")) {
      QString brstr(line.mid(31, 8));
      brstr.remove('(').remove(')');
      double br = clocale.toDouble(brstr.trimmed(), &convok);
      if (!convok)
        br = 1.0;

      QString nbstr(line.mid(41, 8));
      nbstr.remove('(').remove(')');
      double nb = clocale.toDouble(nbstr.trimmed(), &convok);
      if (!convok)
        nb = 1.0;
      normalizeDecIntensToPercentParentDecay = nb * br;

      QString nrstr(line.mid(9, 10));
      nrstr.remove('(').remove(')');
      double nr = clocale.toDouble(nrstr.trimmed(), &convok);
      if (!convok)
        nr = 1.0;
      normalizeGammaIntensToPercentParentDecay = nr * br;
    }
    else if (line.startsWith(dNucid + " PN ")) {
      QString nbbrstr(line.mid(41, 8));
      nbbrstr.remove('(').remove(')');
      double nbbr = clocale.toDouble(nbbrstr.trimmed(), &convok);
      if (convok)
        normalizeDecIntensToPercentParentDecay = nbbr;

      QString nrbrstr(line.mid(9, 10));
      nrbrstr.remove('(').remove(')');
      double nrbr = clocale.toDouble(nrbrstr.trimmed(), &convok);
      if (convok)
        normalizeGammaIntensToPercentParentDecay = nrbr;
    }
  }

  // create relevant parent levels and collect parent half-lifes
  QMap<Energy, EnergyLevel*> plevels;
  std::vector<HalfLife> pHl;
  foreach (ParentRecord p, decaydata.parents) {
    pHl.push_back(p.hl);

    EnergyLevel *plv = new EnergyLevel(p.energy, p.spin, p.hl);
    plv->setFeedingLevel(true);
    plevels.insert(p.energy, plv);
  }
  if (!plevels.isEmpty()) {
    if (plevels.begin().value()->energy() > 0.0) {
      EnergyLevel *plv = new EnergyLevel(Energy(0.0, UncertainDouble::SignMagnitudeDefined), SpinParity(), HalfLife());
      plv->setFeedingLevel(false);
      plevels.insert(Energy(0.0, UncertainDouble::SignMagnitudeDefined), plv);
    }
  }

  Nuclide *pNuc = new Nuclide(decaydata.parents.value(0).nuclide, pHl);
  pNuc->addLevels(plevels);
  Nuclide *dNuc = new Nuclide(decaydata.daughter);
  dNuc->addLevels(levels);

  return QSharedPointer<Decay>(new Decay(QString::fromStdString(decayName), pNuc, dNuc, decaydata.decayType));
}

UncertainDouble ENSDFParser::parseEnsdfMixing(const QString &mstr, const QString &multipolarity)
{
  Q_ASSERT(mstr.size() == 14);

  // special case for pure multipolarities
  if (mstr.trimmed().isEmpty()) {
    QString tmp(multipolarity);
    tmp.remove('(').remove(')');
    if (tmp.count() == 2)
      return UncertainDouble(0.0, 1, UncertainDouble::SignMagnitudeDefined);
    else
      return UncertainDouble();
  }

  // mixed case
//  return parseUncertainty(mstr.left(8).trimmed(), mstr.right(6).trimmed().replace(' ', ""));
  return UncertainDouble::from_nsdf(mstr.left(8).toStdString(), mstr.right(6).replace(' ', "").toStdString());

}

/**
 * @brief ENSDFParser::insertAdoptedLevelsBlock
 * @param adoptblocks map of adopted levels (target)
 * @param newblock block to add
 * @param dssym Symbol of the current decay. Used to filter levels and adjust energies according to XREF records
 */
void ENSDFParser::insertAdoptedLevelsBlock(QMap<Energy, StringSubList> *adoptblocks, const StringSubList &newblock, char dssym) const
{
  Q_ASSERT(adoptblocks);
  // get xref record

  std::list<std::string> req;
  req.push_back("XREF");
  std::vector<std::string> xreflist = extractContinuationRecords(newblock, req);
  QString xref = QString::fromStdString(xreflist.at(0));

  // filter data sets

  // -(AB) case (do not add level if dssym is contained in the parentheses)
  if (xref.startsWith("-(") && xref.endsWith(")") && xref.contains(dssym))
    return;

  // exit if xref is neither "+" (level valid for all datasets) nor -(...) not containing dssymb
  // nor contains dssym
  if (xref != "+" && !xref.startsWith("-(") && !xref.contains(dssym))
    return;

  // if this point is reached the level will be added in any case

  // read energy from level record
  Energy e = Energy::from_nsdf(contents.at(newblock.first).substr(9, 12));


  // for the A(E1) case the energy must be modified
  QRegExp er(QString(dssym) + "\\(([.\\d]+)\\)");
  if (er.indexIn(xref) >= 0) {
    Energy matchedE = Energy::from_nsdf(er.cap(1).toStdString());
    DBG << "ENERGY FROM REGEXP " << er.cap(1).toStdString() << " --> " << matchedE.to_string() << " valid " << matchedE.isValid();

    //std::cerr << "Current xref: " << xref.toStdString() << " current dssymb: " << dssym << std::endl;
    //std::cerr << "Energy translation, old: " << e << " new: " << matchedE << std::endl;
    if (matchedE.isValid())
      e = matchedE;
  }

  Q_ASSERT(newblock.last >= newblock.first);

  // add record
  adoptblocks->insert(e, newblock);
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
T ENSDFParser::findNearest(const QMap<Energy, T> &map, const Energy &val, Energy *foundVal) const
{
  Q_ASSERT(!map.isEmpty());

  typename QMap<Energy, T>::const_iterator i = map.lowerBound(val);

  if (i == map.end())
    i--;
  else if (i != map.begin())
    if (qAbs(val - (i-1).key()) < qAbs(val - i.key()))
      i--;

  if (foundVal)
    (*foundVal) = i.key();

  return i.value();
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
          decaydata.parents.append(ParentRecord::from_ensdf(contents.at(i)));
//          DBG << "  Parent         " << decaydata.parents.back().to_string();
        }
//        else
//          DBG << "             DEC " << contents.at(i);
      }

      if (decaydata.parents.isEmpty()) {
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
      decayname += " → "
                + Decay::decayTypeAsText(decaydata.decayType).toStdString();
                + ", " + join(hlstrings, " + ");

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

