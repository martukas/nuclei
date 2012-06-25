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

    parseContents();
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
    QStringList result = contents.filter(QRegExp("^.{5,5}    .*(B-|B\\+|EC|IT|A\\s)\\sDECAY.*$"));
    for (int i=0; i<result.size(); i++) {
        result[i][4] = (result[i][4]).toLower();
        result[i] = result[i].replace(QRegExp("^\\s*([0-9]+)([A-Za-z]+).*$"), "\\2-\\1");
    }

    result.removeDuplicates();
    return result;
}

QList< QSharedPointer<Decay> > ENSDFMassChain::decays(const QString &daughterNuclide) const
{
    // recover nuclide identification (NUCID)
    QString nucid(nuclideToNucid(daughterNuclide));
    nucid.replace(' ', "\\s");

    // find adopted levels block
    QStringList al;
    QString alregexp("^" + nucid + "    ADOPTED LEVELS.*$");
    int alstart = contents.indexOf(QRegExp(alregexp));
    if (alstart >= 0) {
        int alstop = contents.indexOf(QRegExp("^\\s*$"), alstart);
        if (alstop < 0)
            alstop = contents.size();
        al = contents.mid(alstart, alstop-alstart);
    }

    // find decay blocks
    QList< QSharedPointer<Decay> > result;
    int start = -1, stop = 0;
    QRegExp re("^" + nucid + "\\s{4,4}[0-9]{1,3}[A-Z]{1,2}\\s+(B-|B\\+|EC|IT|A)\\sDECAY.*");
    while ((start = contents.indexOf(re, stop)), start >= 0) {
        stop = contents.indexOf(QRegExp("^\\s*$"), start);
        if (stop < 0)
            stop = contents.size();
        QStringList dec(contents.mid(start, stop-start));
        if (dec.isEmpty())
            continue;
        result.append(QSharedPointer<Decay>(new Decay(dec, al)));
    }
    return result;
}

QSharedPointer<Decay> ENSDFMassChain::decay(const QString &daughterNuclide, const QString &decay)
{

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

void ENSDFMassChain::parseContents()
{
    // create list of block boundaries
    // end index points behind last line of block!
    QList< QPair<int, int> > bb;
    int from = 0;
    QRegExp emptyline("^\\s*$");
    int bidx = contents.indexOf(emptyline, from);
    while (bidx > 0) {
        if (bidx-from > 1)
            bb.append(BlockIndices(from, bidx));
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

            QStringList precstrings = QStringList(contents.mid(block.first, block.second - block.first)).filter(QRegExp("^[\\s0-9A-Z]{5,5}\\s\\sP[\\s0-9].*$"));
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

