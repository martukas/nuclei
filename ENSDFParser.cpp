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
                xrefs.insert(line.mid(9, 30).trimmed(), line.at(8).toAscii());
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
    for (int k=0; k<decaylines.size(); k++) {
        const QString &line(decaylines.at(k));

        // process new gamma
        if ((levels.size() > 1) && line.startsWith(dNucid + "  G ")) {

            Q_ASSERT(!levels.isEmpty());

            // determine energy
            Energy e = parseEnsdfEnergy(line.mid(9, 10));

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
                    Energy gk(parseEnsdfEnergy((*it).mid(9, 10)));
                    if (gk.isValid())
                        e2g.insert(gk, (*it));
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
                EnergyLevel *destlvl = findNearest(levels, Energy(start->energy() - e));

                // gamma registers itself with the start and dest levels
                new GammaTransition(e, in, mpol, delta, start, destlvl);
            }
        }
        // process new level
        else if (line.startsWith(dNucid + "  L ")) {
            // determine energy
            Energy e = ENSDFParser::parseEnsdfEnergy(line.mid(9, 10));
            // determine spin
            SpinParity spin(parseSpinParity(line.mid(21, 18)));
            // determine isomer number
            QString isostr(line.mid(77,2));
            unsigned int isonum = isostr.mid(1,1).toUInt(&convok);
            if (!convok && isostr.at(0) == 'M')
                isonum = 1;
            // determine half-life
            HalfLife hl(parseHalfLife(line.mid(39, 10)));

            // get additional data from adopted leves record
            //   find closest entry
            if (!adoptblocks.isEmpty()) {
                Energy foundE;
                currentadoptblock.clear();
                currentadoptblock = findNearest(adoptblocks, e, &foundE);
                Q_ASSERT(currentadoptblock.last >= currentadoptblock.first);
                if (qAbs(e - foundE) > (adoptedLevelMaxDifference/1000.0*e))
                    currentadoptblock.clear();
            }
            else {
                currentadoptblock.clear();
                Q_ASSERT(currentadoptblock.last >= currentadoptblock.first);
            }

            UncertainDouble Q, mu;

            // if an appropriate entry was found, read its contents
            // set half life if necessary
            if (currentadoptblock.first != currentadoptblock.last) {
                if (!hl.isValid()) {
                    QString levelfirstline(*(currentadoptblock.first));
                    hl = HalfLife(parseHalfLife(levelfirstline.mid(39, 10)));
                    if (!spin.isValid())
                        spin = parseSpinParity(levelfirstline.mid(21, 18));
                }
                // parse continuation records
                QStringList moms = extractContinuationRecords(currentadoptblock, momentsRequestList);
                Q = parseMoment(moms.value(0).trimmed());
                mu = parseMoment(moms.value(1).trimmed());
            }

            currentLevel = new EnergyLevel(e, spin, hl, isonum, Q, mu);
            levels.insert(e, currentLevel);
        }
        // process decay information
        else if (!levels.isEmpty() && line.startsWith(dNucid + "  E ")) {
            double intensity = 0.0;
            bool cok1, cok2;
            QString iestr(line.mid(31, 8));
            iestr.remove('(').remove(')');
            double ie = clocale.toDouble(iestr.trimmed(), &cok1);
            if (cok1)
                intensity += ie * normalizeDecIntensToPercentParentDecay;
            QString ibstr(line.mid(21, 8));
            ibstr.remove('(').remove(')');
            double ib = clocale.toDouble(ibstr.trimmed(), &cok2);
            if (cok2)
                intensity += ib * normalizeDecIntensToPercentParentDecay;
            if (cok1 || cok2)
                currentLevel->setFeedIntensity(intensity);
        }
        else if (!levels.isEmpty() && line.startsWith(dNucid + "  B ")) {
            QString ibstr(line.mid(21, 8));
            ibstr.remove('(').remove(')');
            double ib = clocale.toDouble(ibstr.trimmed(), &convok);
            if (convok)
                currentLevel->setFeedIntensity(ib * normalizeDecIntensToPercentParentDecay);
        }
        else if (!levels.isEmpty() && line.startsWith(dNucid + "  A ")) {
            QString iastr(line.mid(21, 8));
            iastr.remove('(').remove(')');
            double ia = clocale.toDouble(iastr.trimmed(), &convok);
            if (convok)
                currentLevel->setFeedIntensity(ia * normalizeDecIntensToPercentParentDecay);
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
            EnergyLevel *plv = new EnergyLevel(Energy(0.0), SpinParity(), HalfLife());
            plv->setFeedingLevel(false);
            plevels.insert(Energy(0.0), plv);
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
    nucid.append(Nuclide::nameOf(nuclide.second).toUpper().leftJustified(2, ' '));
    return nucid;
}

Nuclide::Coordinates ENSDFParser::nucidToNuclide(const QString &nucid)
{
    if (nucid.size() != 5)
        Nuclide::Coordinates(0, 0);

    return Nuclide::Coordinates(nucid.left(3).trimmed().toUInt(), Nuclide::zOf(nucid.right(2).trimmed()));
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

Energy ENSDFParser::parseEnsdfEnergy(const QString &estr)
{
    QLocale clocale("C");
    QString tmp(estr);
    tmp.remove('(').remove(')');
    tmp.remove("+X"); // fix modified energy values (illegaly used in ensdf...)
    bool convok = false;
    double result = clocale.toDouble(tmp.trimmed(), &convok);
    if (!convok)
        result = std::numeric_limits<double>::quiet_NaN();
    return Energy(result);
}

HalfLife ENSDFParser::parseHalfLife(const QString &hlstr)
{
    double sec = std::numeric_limits<double>::quiet_NaN();
    bool uncert = false;

    QString tstr(hlstr);
    if (tstr.contains('?'))
        uncert = true;
    tstr.remove('?');
    tstr.remove('(').remove(')');
    QStringList timeparts = tstr.trimmed().split(' ');
    if (tstr.contains("STABLE", Qt::CaseInsensitive)) {
        sec = std::numeric_limits<double>::infinity();
    }
    else if (tstr.contains("EV")) {
        sec = std::numeric_limits<double>::quiet_NaN();
    }
    else if (timeparts.size() >= 2) {
        QLocale clocale("C");
        bool ok = false;
        sec = clocale.toDouble(timeparts.at(0), &ok);
        if (!ok)
            sec = std::numeric_limits<double>::infinity();
        else if (timeparts.at(1) == "Y")
            sec *= 365. * 86400.;
        else if (timeparts.at(1) == "D")
            sec *= 86400.;
        else if (timeparts.at(1) == "H")
            sec *= 3600.;
        else if (timeparts.at(1) == "M")
            sec *= 60.;
        else if (timeparts.at(1) == "MS")
            sec *= 1.E-3;
        else if (timeparts.at(1) == "US")
            sec *= 1.E-6;
        else if (timeparts.at(1) == "NS")
            sec *= 1.E-9;
        else if (timeparts.at(1) == "PS")
            sec *= 1.E-12;
        else if (timeparts.at(1) == "FS")
            sec *= 1.E-15;
        else if (timeparts.at(1) == "AS")
            sec *= 1.E-18;
    }

    return HalfLife(sec, uncert);
}

SpinParity ENSDFParser::parseSpinParity(const QString &sstr)
{
    bool weakarg = false;
    SpinParity::Parity p = SpinParity::Undefined;
    unsigned int num = 0, denom = 1;
    bool valid = false;

    QString spstr(sstr.trimmed());
    if (spstr.contains('('))
        weakarg = true;
    spstr.remove('(').remove(')');
    spstr = spstr.trimmed();
    if (spstr.right(1) == "+")
        p = SpinParity::Plus;
    else if (spstr.right(1) == "-")
        p = SpinParity::Minus;
    spstr.remove('+').remove('-');
	if (!spstr.isEmpty() && !spstr.contains(",")) {
        QStringList fract(spstr.split('/'));
        if (!fract.isEmpty()) {
            num = fract.at(0).toUInt();
            if (fract.size() > 1)
                denom = fract.at(1).toUInt();
            if (denom == 0)
                denom = 1;
            valid = true;
        }
    }

    return SpinParity(num, denom, p, weakarg, valid, (valid ? "" : sstr.trimmed()));
}

UncertainDouble ENSDFParser::parseEnsdfMixing(const QString &mstr, const QString &multipolarity)
{
    Q_ASSERT(mstr.size() == 14);

    // special case for pure multipolarities
    if (mstr.trimmed().isEmpty()) {
        QString tmp(multipolarity);
        tmp.remove('(').remove(')');
        if (tmp.count() == 2)
            return UncertainDouble(0.0, UncertainDouble::SignMagnitudeDefined);
        else
            return UncertainDouble();
    }

    // mixed case
    return parseUncertainty(mstr.left(8).trimmed(), mstr.right(6).trimmed().replace(' ', ""));
}

ENSDFParser::ParentRecord ENSDFParser::parseParentRecord(const QString &precstr)
{
    Q_ASSERT(precstr.size() >= 50);

    ParentRecord prec;

    // determine parent data
    prec.nuclide = nucidToNuclide(precstr.left(5));

    // determine parent's half-life
    prec.hl = HalfLife(parseHalfLife(precstr.mid(39, 10)));

    // determine decaying level's energy
    prec.energy = Energy(parseEnsdfEnergy(precstr.mid(9, 10)));

    // determine parent level's spin
    prec.spin = SpinParity(parseSpinParity(precstr.mid(21, 18)));

    return prec;
}

UncertainDouble ENSDFParser::parseMoment(const QString &s)
{
    // MOMXY= must be stripped and the remaining text trimmed before calling this method!

    if (s.isEmpty())
        return UncertainDouble();

    UncertainDouble result;
    QString str(s);
    str.remove('(').remove(')');

    QStringList parts = str.split(" ");
    Q_ASSERT(parts.count() >= 1);
    if (parts.count() == 1)
        parts.prepend("AP");

    QString &first = parts[0];
    if (first == "LT" || first == "GT" || first == "LE" || first == "GE" || first == "AP" || first == "CA" || first == "SY")
        return parseUncertainty(parts.at(1), parts.at(0));

    QString uncert(parts.at(1));
    if (uncert.contains(QRegExp("[A-Za-z]"))) {
        std::cerr << "Invalid or missing Momentum uncertainty: " << uncert.toStdString() << std::endl;
        uncert = "AP";
    }
    return parseUncertainty(parts.at(0), uncert);
}

UncertainDouble ENSDFParser::parseUncertainty(const QString &value, const QString &uncertaintyString)
{
    QString v(value.trimmed());

    if (value.trimmed().isEmpty())
        return UncertainDouble();

    QLocale clocale("C");
    bool convok = false;

    UncertainDouble result(clocale.toDouble(v, &convok), UncertainDouble::UndefinedSign);
    if (convok) {
        if (v.contains('+') || v.contains('-'))
            result.setSign(UncertainDouble::SignMagnitudeDefined);
        else
            result.setSign(UncertainDouble::MagnitudeDefined);

        // parse uncertainty
        // symmetric or special case (consider symmetric if not + and - are both contained in string)
        if ( !(uncertaintyString.contains('+')&&uncertaintyString.contains('-')) ) {
            if (uncertaintyString == "LT")
                result.setUncertainty(-std::numeric_limits<double>::infinity(), 0.0, UncertainDouble::LessThan);
            else if (uncertaintyString == "GT")
                result.setUncertainty(0.0, std::numeric_limits<double>::infinity(), UncertainDouble::GreaterThan);
            else if (uncertaintyString == "LE")
                result.setUncertainty(-std::numeric_limits<double>::infinity(), 0.0, UncertainDouble::LessEqual);
            else if (uncertaintyString == "GE")
                result.setUncertainty(0.0, std::numeric_limits<double>::infinity(), UncertainDouble::GreaterEqual);
            else if (uncertaintyString == "AP")
                result.setUncertainty(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), UncertainDouble::Approximately);
            else if (uncertaintyString == "CA")
                result.setUncertainty(0.0, 0.0, UncertainDouble::Calculated);
            else if (uncertaintyString == "SY")
                result.setUncertainty(0.0, 0.0, UncertainDouble::Systematics);
            else {
                // determine significant figure
                unsigned int uncert = clocale.toUInt(uncertaintyString, &convok);
                if (convok)
                    result.setSymmetricUncertainty(getUncertainty(v, uncert));
                else
                    result.setUncertainty(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), UncertainDouble::UndefinedType);
            }
        }
        // asymmetric case
        else {
            Q_ASSERT(uncertaintyString.contains('+') && uncertaintyString.contains('-'));
            QRegExp re("^\\+([0-9]+)\\-([0-9]+)$");
            QString uposstr, unegstr;
            int pos = re.indexIn(uncertaintyString);
            if (pos > -1) {
                uposstr = re.cap(1);
                unegstr = re.cap(2);
            }
            else {
                QRegExp reinv("^\\-([0-9]+)\\+([0-9]+)$");
                pos = reinv.indexIn(uncertaintyString);
                Q_ASSERT(pos > -1);
                uposstr = reinv.cap(2);
                unegstr = reinv.cap(1);
            }
            Q_ASSERT(uposstr.size() > 0 && unegstr.size() > 0);
            unsigned int upositive = clocale.toUInt(uposstr, &convok);
            bool tmp = convok;
            unsigned int unegative = clocale.toUInt(unegstr, &convok);
            if (tmp && convok) {
                // work aournd bad entries with asymmetric uncertainty values of 0.
                if (upositive == 0.0 || unegative == 0.0) {
                    result.setUncertainty(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), UncertainDouble::Approximately);
                    std::cerr << "Found asymmetric error of 0 in '" << uncertaintyString.toStdString() << "'. Auto-changing to 'approximately'" << std::endl;
                }
                else {
                    result.setAsymmetricUncertainty(getUncertainty(v, unegative), getUncertainty(v, upositive));
                }
            }
            else {
                result.setUncertainty(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), UncertainDouble::UndefinedType);
            }
        }

        return result;
    }
    // else return undefined
    return UncertainDouble();
}

double ENSDFParser::getUncertainty(const QString value, unsigned int stdUncertainty)
{
    QLocale clocale("C");
    bool convok = true;

    const QStringList splitval = value.split('E');
    Q_ASSERT(splitval.size() > 0);
    const QString &mantissa = splitval.value(0);
    const QString &expstr = splitval.value(1); // zero if splitval.size() == 1
    int exponent = 0;
    if (!expstr.isEmpty())
        exponent = clocale.toInt(expstr, &convok);
    Q_ASSERT(convok);

    int sigpos = 0;
    // determine position of . in mantissa
    int pointpos = mantissa.indexOf('.');
    if (pointpos >= 0)
        sigpos -= mantissa.size() - pointpos - 1;

    // return shifted according to exponent
    return stdUncertainty * pow(10.0, double(sigpos + exponent));
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

    // exit if xref is neither "+" (level valid for all datasets) nor contains dssym
    if (xref != "+" && !xref.contains(dssym))
        return;

    // if this point is reached the level will be added in any case

    // read energy from level record
    Energy e = parseEnsdfEnergy((*newblock.first).mid(9, 10));

    // for the A(E1) case the energy must be modified
    QRegExp er(QString(dssym) + "\\(([.\\d]+)\\)");
    if (er.indexIn(xref) >= 0) {
        Energy matchedE = parseEnsdfEnergy(er.cap(1));
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
    // remove record id
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
                hlstrings.append(prec.hl.toString());
            }

            const ParentRecord &prec(decaydata.parents.at(0));
            QString decayname(Nuclide::nameOf(prec.nuclide));
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

