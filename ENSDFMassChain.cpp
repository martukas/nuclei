#include "ENSDFMassChain.h"
#include <QDir>
#include <QFile>
#include <QChar>
#include <QMap>
#include <QLocale>
#include <QSettings>
#include "Decay.h"

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

QSharedPointer<Decay> ENSDFMassChain::decay(const QString &daughterNuclideName, const QString &decayName)
{
    BlockIndices al = m_adoptedlevels.value(daughterNuclideName);
    BlockIndices de = m_decays.value(daughterNuclideName).value(decayName);
    return QSharedPointer<Decay> dec(new Decay);

    double normalizeDecIntensToPercentParentDecay = 1.0;
    double normalizeGammaIntensToPercentParentDecay = 1.0;

    // process all level sub-blocks
    EnergyLevel *currentLevel = 0;
    QLocale clocale("C");
    bool convok;

    // create index for adopted levels
    QMap<double, QStringList> adoptblocks;
    int laststart = -1;
    for (int i=al.first; i < al.first+al.second; i++) {
        const QString &line = contents.at(i);
        if (line.startsWith(dNuc.nucid() + "  L ")) {
            if (laststart > 0)
                adoptblocks.insert(parseEnsdfEnergy(contents.at(laststart).mid(9, 10)),
                                   QStringList(contents.mid(laststart, i-laststart)));
            laststart = i;
        }
    }
    if (laststart > 0)
        adoptblocks.insert(ENSDFMassChain::parseEnsdfEnergy(contents.at(laststart).mid(9, 10)),
                           QStringList(contents.mid(laststart, al.second-laststart)));


    foreach (const QString &line, QStringList(contents.mid(de.first, de.second))) {

        // process new gamma
        if (!dec->levels.isEmpty() && line.startsWith(dNuc.nucid() + "  G ")) {

            Q_ASSERT(!levels.isEmpty());

            // determine energy
            double e = ENSDFMassChain::parseEnsdfEnergy(line.mid(9, 10));

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
                QStringList adptlvl(selectAdoptedLevelsDataBlock(currentLevel->energyKeV()));
                // filter gamma records
                QRegExp gammare("^" + dNuc.nucid() + "  G (.*)$");
                adptlvl = adptlvl.filter(gammare);
                // create gamma map
                QMap<double, QString> e2g;
                foreach (QString g, adptlvl) {
                    double gk = clocale.toDouble(g.mid(9, 10), &convok);
                    if (convok)
                        e2g.insert(gk, g);
                }
                // find gamma
                double gidx = findNearest(e2g, e);
                if ((e-gidx < gammaMaxDifference/1000.0*e) && std::isfinite(gidx)) {
                    if (mpol.isEmpty())
                        mpol = e2g.value(gidx).mid(31, 10).trimmed();

                    if (deltastate != GammaTransition::SignMagnitudeDefined) {
                        GammaTransition::DeltaState adptdeltastate = GammaTransition::UnknownDelta;
                        double adptdelta = parseEnsdfMixing(e2g.value(gidx).mid(41, 8).trimmed(), mpol, &adptdeltastate);
                        if (adptdeltastate > deltastate) {
                            delta = adptdelta;
                            deltastate = adptdeltastate;
                        }
                    }
                }
            }

            // determine levels
            EnergyLevel *start = currentLevel;
            double destlvlidx = findNearest(levels, start->energyKeV() - e);
            Q_ASSERT(levels.contains(destlvlidx));

            // gamma registers itself with the start and dest levels
            new GammaTransition(e, in, mpol, delta, deltastate, start, levels[destlvlidx]);
        }
        // process new level
        else if (line.startsWith(dNuc.nucid() + "  L ")) {
            // determine energy
            double e = ENSDFMassChain::parseEnsdfEnergy(line.mid(9, 10));
            // determine spin
            SpinParity spin(line.mid(21, 18));
            // determine isomer number
            QString isostr(line.mid(77,2));
            unsigned int isonum = isostr.mid(1,1).toUInt(&convok);
            if (!convok && isostr.at(0) == 'M')
                isonum = 1;
            // determine half-life
            HalfLife hl(line.mid(39, 10));

            // get additional data from adopted leves record
            //   find closest entry
            double Q = std::numeric_limits<double>::quiet_NaN();
            double mu = std::numeric_limits<double>::quiet_NaN();

            QStringList adptlvl(selectAdoptedLevelsDataBlock(e));

            // if an appropriate entry was found, read its contents
            // set half life if necessary
            if (!adptlvl.isEmpty() && !hl.isValid())
                hl = HalfLife(adptlvl.at(0).mid(39, 10));

            // parse continuation records
            // fetch records
            QRegExp crecre("^" + dNuc.nucid() + "[A-RT-Z0-9] L (.*)$");
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
        else if (!levels.isEmpty() && line.startsWith(dNuc.nucid() + "  E ")) {
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
                currentLevel->feedintens = intensity;
        }
        else if (!levels.isEmpty() && line.startsWith(dNuc.nucid() + "  B ")) {
            QString ibstr(line.mid(21, 8));
            ibstr.remove('(').remove(')');
            double ib = clocale.toDouble(ibstr.trimmed(), &convok);
            if (convok)
                currentLevel->feedintens = ib * normalizeDecIntensToPercentParentDecay;
        }
        else if (!levels.isEmpty() && line.startsWith(dNuc.nucid() + "  A ")) {
            QString iastr(line.mid(21, 8));
            iastr.remove('(').remove(')');
            double ia = clocale.toDouble(iastr.trimmed(), &convok);
            if (convok)
                currentLevel->feedintens = ia * normalizeDecIntensToPercentParentDecay;
        }
        // process normalization records
        else if (line.startsWith(dNuc.nucid() + "  N ")) {
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
        else if (line.startsWith(dNuc.nucid() + " PN ")) {
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
}

double ENSDFMassChain::parseEnsdfEnergy(const QString &estr)
{
    QLocale clocale("C");
    QString tmp(estr);
    tmp.remove('(').remove(')');
    return clocale.toDouble(tmp.trimmed());
}

ENSDFMassChain::ParentRecord ENSDFMassChain::parseParentRecord(const QString &precstr)
{
    Q_ASSERT(precstr.size() >= 50);

    ParentRecord prec;

    // determine parent data
    prec.nuclideName = nucidToNuclide(precstr.left(5));

    // determine parent's half-life
    prec.hl = HalfLife(precstr.mid(39, 10));

    // determine decaying level's energy
    prec.energy = parseEnsdfEnergy(precstr.mid(9, 10));

    // determine parent level's spin
    prec.spin = SpinParity(precstr.mid(21, 18));

    return prec;
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

            // get daughter name
            const QString daughtername(nucidToNuclide(decre.capturedTexts().at(1)));

            // get reference to daughter map to work with (create and insert if necessary)
            QMap<QString, QPair<int, int> > &decmap(m_decays[daughtername]);

            // create decay name
            const QString dtypename(Decay::decayTypeAsText(parseDecayType(decre.capturedTexts().at(2))));

            QStringList precstrings = QStringList(contents.mid(block.first, block.second)).filter(QRegExp("^[\\s0-9A-Z]{5,5}\\s\\sP[\\s0-9].*$"));
            QList<ParentRecord> precs;
            foreach (const QString &precstr, precstrings)
                precs.append(parseParentRecord(precstr));

            Q_ASSERT(!precs.isEmpty());
            if (precs.isEmpty())
                continue;

            QStringList hlstrings;
            foreach (const ParentRecord &prec, precs)
                hlstrings.append(prec.hl.toString());

            const ParentRecord &prec(precs.at(0));
            QString decayname(prec.nuclideName);
            if (prec.energy > 0)
                decayname.append("m");
            decayname.append(", ").append(dtypename).append(", ").append(hlstrings.join(" + "));

            // insert
            Q_ASSERT(!decmap.contains(decayname));
            decmap.insert(decayname, block);
        }
    }
}

