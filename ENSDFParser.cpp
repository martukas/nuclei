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

QList<unsigned int> ENSDFParser::aList;

ENSDFParser::ENSDFParser(unsigned int A)
  : a(A)
{
  QSettings s;
  // read data
  QFile f(s.value("ensdfPath", ".").toString() + QString("/ensdf.%1").arg(a, int(3), int(10), QChar('0')));
  f.open(QIODevice::ReadOnly | QIODevice::Text);
  QString c = QString::fromUtf8(f.readAll());

  contents = c.split('\n');

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

const QList<Nuclide::Coordinates> ENSDFParser::daughterNuclides() const
{
  Q_ASSERT(m_decays.size() == m_decays.keys().toSet().size()); // check for duplicates
  return m_decays.keys();
}

const QList< QPair<QString, Nuclide::Coordinates> > ENSDFParser::decays(const Nuclide::Coordinates &daughterNuclide) const
{
  Q_ASSERT(m_decays.value(daughterNuclide).size() == m_decays.value(daughterNuclide).keys().toSet().size()); // check for duplicates
  QList< QPair<QString, Nuclide::Coordinates> > result;
  QMapIterator<QString, BasicDecayData> i(m_decays.value(daughterNuclide));
  while (i.hasNext()) {
    i.next();
    result.append(QPair<QString, Nuclide::Coordinates>(i.key(), i.value().parents.at(0).nuclide));
  }
  return result;
}

QSharedPointer<Decay> ENSDFParser::decay(const Nuclide::Coordinates &daughterNuclide, const QString &decayName) const
{
  QSettings s;
  double adoptedLevelMaxDifference = s.value("preferences/levelTolerance", 1.0).toDouble();
  double gammaMaxDifference = s.value("preferences/gammaTolerance", 1.0).toDouble();

  QStringList momentsRequestList;
  momentsRequestList << "MOME2" << "MOMM1";

  QMap<Energy, EnergyLevel*> levels;

  BlockIndices alpos = m_adoptedlevels.value(daughterNuclide);
  BasicDecayData decaydata = m_decays.value(daughterNuclide).value(decayName);
  double normalizeDecIntensToPercentParentDecay = 1.0;
  double normalizeGammaIntensToPercentParentDecay = 1.0;
  QString dNucid = nuclideToNucid(daughterNuclide);

//  DBG << "parsing " << dNucid.toStdString();

  // process all adopted level sub-blocks
  EnergyLevel *currentLevel = 0;
  QLocale clocale("C");
  bool convok;
  // create index map for adopted levels
  QMap<Energy, StringSubList> adoptblocks;
  QMap<QString, char> xrefs; // maps DSID to DSSYM (single letter)
  int laststart = -1;
  for (int i=alpos.first; i < alpos.first+alpos.second; i++) {
    const QString &line = contents.at(i);
    // extract cross reference records as long as first level was found (cross reference must be before 1st level)
    if (laststart == -1) {
      if (line.startsWith(dNucid + "  X"))
        xrefs.insert(line.mid(9, 30).trimmed(), line.at(8).toLatin1());
    }
    // find level records
    if (line.startsWith(dNucid + "  L ")) {
      if (laststart > 0) {
        StringSubList sl(contents.constBegin() + laststart, contents.constBegin() + i);
        Q_ASSERT(!(sl.last > contents.end()));
        Q_ASSERT(sl.first <= sl.last);
        insertAdoptedLevelsBlock(&adoptblocks, sl, xrefs.value(decaydata.dsid));
      }
      laststart = i;
    }
  }
  if (laststart > 0) {
    StringSubList sl(contents.constBegin() + laststart, contents.constBegin() + alpos.first + alpos.second);
    Q_ASSERT(!(sl.last > contents.end()));
    Q_ASSERT(sl.first <= sl.last);
    insertAdoptedLevelsBlock(&adoptblocks, sl, xrefs.value(decaydata.dsid));
  }

  // adopted levels block of current level in decay data set
  StringSubList currentadoptblock;

  // process decay block
  const QStringList decaylines(contents.mid(decaydata.block.first, decaydata.block.second));

//  DBG << "parsing size " << decaylines.size();

  for (int k=0; k<decaylines.size(); k++) {
    const QString &line(decaylines.at(k));

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
        QStringList::const_iterator it(currentadoptblock.first);
        while (it != currentadoptblock.last) {
          if ((*it).startsWith(dNucid + "  G ")) {
            Energy gk = Energy::from_nsdf((*it).mid(9, 12).toStdString());
            if (gk.isValid())
              e2g.insert(gk, (*it));
          }
          it++;
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
        QString levelfirstline(*(currentadoptblock.first));
        if (!currentLevel->halfLife().isValid()) {
          currentLevel->set_halflife(HalfLife::from_ensdf(levelfirstline.mid(39, 16).toStdString()));
          if (!currentLevel->spin().valid())
            currentLevel->set_spin(SpinParity::from_ensdf(levelfirstline.mid(21, 18).toStdString()));
        }
        // parse continuation records
        QStringList moms = extractContinuationRecords(currentadoptblock, momentsRequestList);
        currentLevel->set_q(Moment::from_ensdf(moms.value(0).toStdString()));
        currentLevel->set_mu(Moment::from_ensdf(moms.value(1).toStdString()));
        DBG << levelfirstline.toStdString()  << " === " << currentLevel->to_string();
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
  QList<HalfLife> pHl;
  foreach (ParentRecord p, decaydata.parents) {
    pHl.append(p.hl);

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

  Nuclide *pNuc = new Nuclide(decaydata.parents.value(0).nuclide.first, decaydata.parents.value(0).nuclide.second, pHl);
  pNuc->addLevels(plevels);
  Nuclide *dNuc = new Nuclide(decaydata.daughter.first, decaydata.daughter.second);
  dNuc->addLevels(levels);

  return QSharedPointer<Decay>(new Decay(decayName, pNuc, dNuc, decaydata.decayType));
}

QString ENSDFParser::nuclideToNucid(Nuclide::Coordinates nuclide)
{
  QString nucid(QString::number(nuclide.first).rightJustified(3, ' '));
  nucid.append(Nuclide::symbolOf(nuclide.second).toUpper().leftJustified(2, ' '));
  return nucid;
}

Nuclide::Coordinates ENSDFParser::nucidToNuclide(const QString &nucid)
{
  if (nucid.size() != 5)
    return Nuclide::Coordinates(0, 0);

  Nuclide::Coordinates ret(nucid.left(3).trimmed().toUInt(), Nuclide::zOfSymbol(nucid.right(2).trimmed()));
  if (ret.second < 0) {
    QString st = "1" + nucid.right(2).trimmed();
    ret.second = st.toInt();
  }

  return ret;
}

Decay::Type ENSDFParser::parseDecayType(const QString &tstring)
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

ENSDFParser::ParentRecord ENSDFParser::parseParentRecord(const QString &precstr)
{
  Q_ASSERT(precstr.size() >= 50);

  ParentRecord prec;

  // determine parent data
  prec.nuclide = nucidToNuclide(precstr.left(5));

  // determine parent's half-life
  prec.hl = HalfLife::from_ensdf(precstr.mid(39, 16).toStdString());

  // determine decaying level's energy
  prec.energy = Energy::from_nsdf(precstr.mid(9, 12).toStdString());


  // determine parent level's spin
  prec.spin = SpinParity::from_ensdf(precstr.mid(21, 18).toStdString());

  return prec;
}

/**
 * @brief ENSDFParser::insertAdoptedLevelsBlock
 * @param adoptblocks map of adopted levels (target)
 * @param newblock block to add
 * @param dssym Symbol of the current decay. Used to filter levels and adjust energies according to XREF records
 */
void ENSDFParser::insertAdoptedLevelsBlock(QMap<Energy, StringSubList> *adoptblocks, const StringSubList &newblock, char dssym)
{
  Q_ASSERT(adoptblocks);
  // get xref record

  QStringList req;
  req << "XREF";
  QStringList xreflist = extractContinuationRecords(newblock, req);
  QString xref = xreflist.value(0);

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
  Energy e = Energy::from_nsdf((*newblock.first).mid(9, 12).toStdString());


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
QStringList ENSDFParser::extractContinuationRecords(const StringSubList &adoptedblock, const QStringList &requestedRecords, char typeOfContinuedRecord)
{
  // fetch records
  QRegExp crecre("^[A-Z0-9\\s]{5,5}[A-RT-Z0-9] " + QString(typeOfContinuedRecord) + " (.*)$");
  QStringList crecs;
  QStringList::const_iterator i(adoptedblock.first);
  while (i != adoptedblock.last) {
    if (i->contains(crecre))
      crecs.append(*i);
    i++;
  }
  // remove record id from beginning of string
  for (int i=0; i<crecs.size(); i++)
    crecs[i].remove(0, 9);
  // join lines and then split records
  QString tmp(crecs.join("$"));
  crecs = tmp.split('$');
  for (int i=0; i<crecs.size(); i++)
    crecs[i] = crecs[i].trimmed();
  // search and parse requested fields
  QStringList result;
  foreach (const QString &req, requestedRecords) {
    QString rstr;
    for (int i=0; i<crecs.size(); i++) {
      if (crecs.at(i).startsWith(req)) {
        rstr = crecs.at(i).mid(5).trimmed();
        break;
      }
    }
    if (rstr.startsWith('='))
      rstr.remove(0, 1);
    result << rstr;
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
  QList< QPair<int, int> > bb;
  int from = 0;
  QRegExp emptyline("^\\s*$");
  int bidx = contents.indexOf(emptyline, from);
  while (bidx > 0) {
    if (bidx-from > 1)
      bb.append(BlockIndices(from, bidx-from));
    from = bidx + 1;
    bidx = contents.indexOf(emptyline, from);
  }
  bb.append(QPair<int, int>(from, contents.size()-1));

  // prepare regexps
  QRegExp alre("^([\\sA-Z0-9]{5,5})    ADOPTED LEVELS.*$");
  QRegExp decre("^([\\sA-Z0-9]{5,5})\\s{4,4}[0-9]{1,3}[A-Z]{1,2}\\s+((?:B-|B\\+|EC|IT|A)\\sDECAY).*$");

  // recognize blocks
  foreach (const BlockIndices &block, bb) {
    const QString &head(contents.value(block.first));

    // adopted levels
    if (alre.exactMatch(head)) {
      m_adoptedlevels.insert(nucidToNuclide(alre.capturedTexts().at(1)), block);
    }

    // decays
    else if (decre.exactMatch(head)) {
      BasicDecayData decaydata;

      decaydata.daughter = nucidToNuclide(decre.capturedTexts().at(1));
      decaydata.decayType = parseDecayType(decre.capturedTexts().at(2));
      decaydata.block = block;
      decaydata.dsid = head.mid(9,30).trimmed(); // saved for comparison with xref records

      QStringList precstrings = QStringList(contents.mid(block.first, block.second)).filter(QRegExp("^[\\s0-9A-Z]{5,5}\\s\\sP[\\s0-9].*$"));
      foreach (const QString &precstr, precstrings)
        decaydata.parents.append(parseParentRecord(precstr));

      if (decaydata.parents.isEmpty()) // broken records. skipping
        continue;

      // create decay string
      //   get reference to daughter map to work with (create and insert if necessary)
      QMap<QString, BasicDecayData > &decmap = m_decays[decaydata.daughter];

      QStringList hlstrings;
      foreach (const ParentRecord &prec, decaydata.parents) {
        Q_ASSERT(prec.nuclide == decaydata.parents.at(0).nuclide); // check "same parent/different half-life" scheme
        hlstrings.append(QString::fromStdString(prec.hl.to_string()));
      }

      const ParentRecord &prec(decaydata.parents.at(0));
      QString decayname(Nuclide::symbolicName(prec.nuclide));
      if (prec.energy > 0.0)
        decayname.append("m");
      decayname.append(QString::fromUtf8(" → "))
          .append(Decay::decayTypeAsText(decaydata.decayType))
          .append(", ")
          .append(hlstrings.join(" + "));

      // insert into decay map
      while (decmap.contains(decayname))
        decayname.append(" (alt.)");
      decmap.insert(decayname, decaydata);
    }
  }
}

