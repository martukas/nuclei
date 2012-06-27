#include "ENSDFMassChain.h"
#include <QDir>
#include <QFile>
#include <QChar>
#include <QMap>
#include <QLocale>
#include <QSettings>
#include <cmath>

#include "Decay.h"
#include "Nuclide.h"
#include "EnergyLevel.h"
#include "GammaTransition.h"

ENSDFMassChain::ENSDFMassChain(int A)
    : a(A)
{
    QSettings s;
    // read data
            QFile f(s.value("ensdfPath", ".").toString() + QString("/ensdf.%1").arg(A, int(3), int(10), QChar('0')));
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    QString c = QString::fromUtf8(f.readAll());

    contents = c.split('\n');

    parseBlocks();
}

QStringList ENSDFMassChain::aValues() // static
{
    QSettings s;
    if (!s.contains("ensdfPath"))
        return QStringList();

    QDir dir(s.value("ensdfPath").toString());
    if (!dir.exists())
        return QStringList();

    QStringList files = dir.entryList(QStringList("ensdf.???"), QDir::Files | QDir::Readable, QDir::Name);

    for (int i=0; i<files.size(); i++)
        files[i] = files.at(i).right(3).remove(QRegExp("^0+"));
    return files;
}

QStringList ENSDFMassChain::daughterNuclides() const
{
    QStringList result(m_decays.keys());
    result.removeDuplicates();
    return result;
}

QStringList ENSDFMassChain::decays(const QString &daughterNuclideName) const
{
    QStringList result(m_decays.value(daughterNuclideName).keys());
    result.removeDuplicates();
    return result;
}

QSharedPointer<Decay> ENSDFMassChain::decay(const QString &daughterNuclideName, const QString &decayName,
                                            double adoptedLevelMaxDifference, double gammaMaxDifference)
{
    QMap<double, EnergyLevel*> levels;

    BlockIndices alpos = m_adoptedlevels.value(daughterNuclideName);
    BasicDecayData decaydata = m_decays.value(daughterNuclideName).value(decayName);
    double normalizeDecIntensToPercentParentDecay = 1.0;
    double normalizeGammaIntensToPercentParentDecay = 1.0;
    QString dNucid = nuclideToNucid(daughterNuclideName);

    // process all adopted level sub-blocks
    EnergyLevel *currentLevel = 0;
    QLocale clocale("C");
    bool convok;
    // create index map for adopted levels
    QMap<double, QStringList> adoptblocks;
    int laststart = -1;
    for (int i=alpos.first; i < alpos.first+alpos.second; i++) {
        const QString &line = contents.at(i);
        if (line.startsWith(dNucid + "  L ")) {
            if (laststart > 0)
                adoptblocks.insert(parseEnsdfEnergy(contents.at(laststart).mid(9, 10)),
                                   QStringList(contents.mid(laststart, i-laststart)));
            laststart = i;
        }
    }
    if (laststart > 0)
        adoptblocks.insert(ENSDFMassChain::parseEnsdfEnergy(contents.at(laststart).mid(9, 10)),
                           QStringList(contents.mid(laststart, alpos.second-laststart)));

    // process decay block
    const QStringList decaylines(contents.mid(decaydata.block.first, decaydata.block.second));
    foreach (const QString &line, decaylines) {

        // process new gamma
        if ((levels.size() > 1) && line.startsWith(dNucid + "  G ")) {

            Q_ASSERT(!levels.isEmpty());

            // determine energy
            double e = parseEnsdfEnergy(line.mid(9, 10));

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
            GammaTransition::DeltaState deltastate = GammaTransition::UnknownDelta;

            double delta = parseEnsdfMixing(line.mid(41, 8).trimmed(), mpol, &deltastate);

            // parse adopted levels if necessary
            if (deltastate != GammaTransition::SignMagnitudeDefined || mpol.isEmpty()) {
                // Get adopted levels block for current level
                QStringList adptlvl;
                if (!adoptblocks.isEmpty()) {
                    double foundE = 0.0;
                    QStringList adoptblock = findNearest(adoptblocks, currentLevel->energyKeV(), &foundE);
                    if (qAbs(currentLevel->energyKeV() - foundE) <= (adoptedLevelMaxDifference/1000.0*currentLevel->energyKeV()))
                        adptlvl = adoptblock;
                }
                // filter gamma records
                QRegExp gammare("^" + dNucid + "  G (.*)$");
                adptlvl = adptlvl.filter(gammare);
                // create gamma map
                QMap<double, QString> e2g;
                foreach (QString g, adptlvl) {
                    double gk = parseEnsdfEnergy(g.mid(9, 10));
                    if (std::isfinite(gk))
                        e2g.insert(gk, g);
                }
                // find gamma
                if (!e2g.isEmpty()) {
                    double foundE = 0.0;
                    const QString gamma = findNearest(e2g, e, &foundE);
                    if (e-foundE < gammaMaxDifference/1000.0*e) {
                        if (mpol.isEmpty())
                            mpol = gamma.mid(31, 10).trimmed();

                        if (deltastate != GammaTransition::SignMagnitudeDefined) {
                            GammaTransition::DeltaState adptdeltastate = GammaTransition::UnknownDelta;
                            double adptdelta = parseEnsdfMixing(gamma.mid(41, 8).trimmed(), mpol, &adptdeltastate);
                            if (adptdeltastate > deltastate) {
                                delta = adptdelta;
                                deltastate = adptdeltastate;
                            }
                        }
                    }
                }
            }

            // determine levels
            if (!levels.isEmpty()) {
                EnergyLevel *start = currentLevel;
                EnergyLevel *destlvl = findNearest(levels, start->energyKeV() - e);

                // gamma registers itself with the start and dest levels
                new GammaTransition(e, in, mpol, delta, deltastate, start, destlvl);
            }
        }
        // process new level
        else if (line.startsWith(dNucid + "  L ")) {
            // determine energy
            double e = ENSDFMassChain::parseEnsdfEnergy(line.mid(9, 10));
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
            double Q = std::numeric_limits<double>::quiet_NaN();
            double mu = std::numeric_limits<double>::quiet_NaN();

            QStringList adptlvl;
            if (!adoptblocks.isEmpty()) {
                double foundE = 0.0;
                const QStringList adoptblock = findNearest(adoptblocks, e, &foundE);
                if (qAbs(e - foundE) <= (adoptedLevelMaxDifference/1000.0*e))
                    adptlvl = adoptblock;
            }

            // if an appropriate entry was found, read its contents
            // set half life if necessary
            if (!adptlvl.isEmpty() && !hl.isValid())
                hl = HalfLife(parseHalfLife(adptlvl.at(0).mid(39, 10)));

            // parse continuation records
            // fetch records
            QRegExp crecre("^" + dNucid + "[A-RT-Z0-9] L (.*)$");
            QStringList crecs(adptlvl.filter(crecre));
            // remove record id
            crecs.replaceInStrings(crecre, "\\1");
            // join lines and then split records
            QString tmp(crecs.join("$"));
            crecs = tmp.split('$');
            for (int i=0; i<crecs.size(); i++)
                crecs[i] = crecs[i].trimmed();
            // search and parse Q and µ fields
            // Q
            QString qstr(crecs.value(crecs.indexOf(QRegExp("^MOME2.*$"))));
            qstr.replace(QRegExp("^MOME2\\s*=\\s*([\\S]+).*"), "\\1");
            qstr.remove('(').remove(')');
            Q = clocale.toDouble(qstr, &convok);
            if (!convok)
                Q = std::numeric_limits<double>::quiet_NaN();
            // µ
            QString mustr(crecs.value(crecs.indexOf(QRegExp("^MOMM1.*$"))));
            mustr.replace(QRegExp("^MOMM1\\s*=\\s*([\\S]+).*"), "\\1");
            mustr.remove('(').remove(')');
            mu = clocale.toDouble(mustr, &convok);
            if (!convok)
                mu = std::numeric_limits<double>::quiet_NaN();

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
    QMap<double, EnergyLevel*> plevels;
    QList<HalfLife> pHl;
    foreach (ParentRecord p, decaydata.parents) {
        pHl.append(p.hl);

        EnergyLevel *plv = new EnergyLevel(p.energy, p.spin, p.hl);
        plv->setFeedingLevel(true);
        plevels.insert(p.energy, plv);
    }
    if (!plevels.isEmpty()) {
        if (plevels.begin().value()->energyKeV() > 0.0) {
            EnergyLevel *plv = new EnergyLevel(0.0, SpinParity(), HalfLife());
            plv->setFeedingLevel(false);
            plevels.insert(0.0, plv);
        }
    }

    Nuclide *pNuc = new Nuclide(A(decaydata.parents.value(0).nuclideName), element(decaydata.parents.value(0).nuclideName), pHl);
    pNuc->addLevels(plevels);
    Nuclide *dNuc = new Nuclide(A(decaydata.daughterName), element(decaydata.daughterName));
    dNuc->addLevels(levels);
    return QSharedPointer<Decay>(new Decay(decayName, pNuc, dNuc, decaydata.decayType));
}

QString ENSDFMassChain::nuclideToNucid(const QString &nuclide)
{
    QStringList parts = nuclide.split('-');
    if (parts.size() < 2)
        return QString();

    QString nucid(parts.at(1).rightJustified(3, ' '));
    nucid.append(parts.at(0).toUpper().leftJustified(2, ' '));
    return nucid;
}

QString ENSDFMassChain::nucidToNuclide(const QString &nucid)
{
    if (nucid.size() != 5)
        return QString();

    QString nuclide("-" + nucid.left(3).trimmed());
    QString el(nucid.right(2).trimmed());
    if (el.size() > 1)
        el[1] = el.at(1).toLower();
    nuclide.prepend(el);
    return nuclide;
}

unsigned int ENSDFMassChain::A(const QString &nuclide)
{
    return nuclide.split("-").value(1).toUInt();
}

QString ENSDFMassChain::element(const QString &nuclide)
{
    return nuclide.split("-").value(0);
}

Decay::Type ENSDFMassChain::parseDecayType(const QString &tstring)
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

double ENSDFMassChain::parseEnsdfEnergy(const QString &estr)
{
    QLocale clocale("C");
    QString tmp(estr);
    tmp.remove('(').remove(')');
    bool convok = false;
    double result = clocale.toDouble(tmp.trimmed(), &convok);
    if (!convok)
        result = std::numeric_limits<double>::quiet_NaN();
    return result;
}

HalfLife ENSDFMassChain::parseHalfLife(const QString &hlstr)
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

SpinParity ENSDFMassChain::parseSpinParity(const QString &sstr)
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
    QStringList fract(spstr.split('/'));
    if (!fract.isEmpty()) {
        num = fract.at(0).toUInt();
        if (fract.size() > 1)
            denom = fract.at(1).toUInt();
        if (denom == 0)
            denom = 1;
        valid = true;
    }

    return SpinParity(num, denom, p, weakarg, valid);
}

double ENSDFMassChain::parseEnsdfMixing(const QString &mstr, const QString &multipolarity, GammaTransition::DeltaState *state)
{
    QLocale clocale("C");
    bool convok = false;
    double delta = 0.0;
    if (mstr.isEmpty()) {
        QString tmp(multipolarity);
        tmp.remove('(').remove(')');
        if (tmp.count() == 2)
            *state = GammaTransition::SignMagnitudeDefined;
        // else leave deltastate UnknownDelta
    }
    else {
        double tmp = clocale.toDouble(mstr, &convok);
        if (convok) {
            delta = tmp;
            if (mstr.contains('+') || mstr.contains('-'))
                *state = GammaTransition::SignMagnitudeDefined;
            else
                *state = GammaTransition::MagnitudeDefined;
        }
        // else leave deltastate UnknownDelta
    }
    return delta;
}

ENSDFMassChain::ParentRecord ENSDFMassChain::parseParentRecord(const QString &precstr)
{
    Q_ASSERT(precstr.size() >= 50);

    ParentRecord prec;

    // determine parent data
    prec.nuclideName = nucidToNuclide(precstr.left(5));

    // determine parent's half-life
    prec.hl = HalfLife(parseHalfLife(precstr.mid(39, 10)));

    // determine decaying level's energy
    prec.energy = parseEnsdfEnergy(precstr.mid(9, 10));

    // determine parent level's spin
    prec.spin = SpinParity(parseSpinParity(precstr.mid(21, 18)));

    return prec;
}

template <typename T>
T & ENSDFMassChain::findNearest(QMap<double, T> &map, double val, double *foundVal)
{
    Q_ASSERT(!map.isEmpty());

    typename QMap<double, T>::iterator i = map.lowerBound(val);

    if (i == map.begin())
        return i.value();

    if (qAbs(val - (i-1).key()) < qAbs(val - i.key()))
        return (i-1).value();

    if (foundVal)
        *foundVal = i.key();

    return i.value();
}

void ENSDFMassChain::parseBlocks()
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
    bb.append(QPair<int, int>(from, contents.size()));

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

            decaydata.daughterName = nucidToNuclide(decre.capturedTexts().at(1));
            decaydata.decayType = parseDecayType(decre.capturedTexts().at(2));
            decaydata.block = block;

            QStringList precstrings = QStringList(contents.mid(block.first, block.second)).filter(QRegExp("^[\\s0-9A-Z]{5,5}\\s\\sP[\\s0-9].*$"));
            foreach (const QString &precstr, precstrings)
                decaydata.parents.append(parseParentRecord(precstr));

            Q_ASSERT(!decaydata.parents.isEmpty());
            if (decaydata.parents.isEmpty())
                continue;

            // create decay string
            //   get reference to daughter map to work with (create and insert if necessary)
            QMap<QString, BasicDecayData > &decmap(m_decays[decaydata.daughterName]);

            QStringList hlstrings;
            foreach (const ParentRecord &prec, decaydata.parents) {
                Q_ASSERT(prec.nuclideName == decaydata.parents.at(0).nuclideName);
                hlstrings.append(prec.hl.toString());
            }

            const ParentRecord &prec(decaydata.parents.at(0));
            QString decayname(prec.nuclideName);
            if (prec.energy > 0)
                decayname.append("m");
            decayname.append(", ")
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

