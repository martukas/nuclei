#include "ENSDF.h"
#include <QDir>
#include <QFile>
#include <QChar>
#include <QMap>
#include <QLocale>
#include <QSettings>
#include "Decay.h"

QStringList ENSDF::name() const
{
    return contents;
}

ENSDF::ENSDF(int A)
    : a(A)
{
    QSettings s;
    // read data
            QFile f(s.value("ensdfPath", ".").toString() + QString("/ensdf.%1").arg(A, int(3), int(10), QChar('0')));
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    QString c = QString::fromUtf8(f.readAll());

    contents = c.split('\n');


}

QStringList ENSDF::aValues() // static
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

QStringList ENSDF::daughterNuclides() const
{
    QStringList result = contents.filter(QRegExp("^.{5,5}    .*(B-|B\\+|EC|IT|A\\s)\\sDECAY.*$"));
    for (int i=0; i<result.size(); i++) {
        result[i][4] = (result[i][4]).toLower();
        result[i] = result[i].replace(QRegExp("^\\s*([0-9]+)([A-Za-z]+).*$"), "\\2-\\1");
    }

    result.removeDuplicates();
    return result;
}

/**
 * Receiver is responsible for deleting the returned objects
 */
QList< QSharedPointer<Decay> > ENSDF::decays(const QString &daughterNuclide) const
{
    // recover nuclide identification (NUCID)
    QStringList parts = daughterNuclide.split('-');
    if (parts.size() < 2)
        return QList< QSharedPointer<Decay> >();
    QString nucid(parts.at(1).rightJustified(3, ' '));
    nucid.append(parts.at(0).toUpper().leftJustified(2, ' '));
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
    QRegExp re("^" + nucid + "\\s{4,4}[0-9]{1,3}[\\sA-Z]{1,2}\\s(B-|B\\+|EC|IT|A\\s)\\sDECAY.*");
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

