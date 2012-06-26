#include "Nuclide.h"
#include <QGraphicsItemGroup>
#include <QGraphicsSimpleTextItem>
#include <QFontMetrics>
#include <QBrush>

Nuclide::Nuclide()
    : m_A(0), item(0)
{
}

Nuclide::Nuclide(unsigned int A, const QString &element, const HalfLife &halfLife)
    : m_A(A), el(element), item(0)
{
    hl.append(halfLife);
    el = element.toLower();
    if (!el.isEmpty())
        el[0] = el[0].toUpper();
}

Nuclide::Nuclide(unsigned int A, const QString &element, const QList<HalfLife> &halfLifes)
    : m_A(A), el(element), hl(halfLifes), item(0)
{
}

unsigned int Nuclide::a() const
{
    return m_A;
}

unsigned int Nuclide::z() const
{
    return elToZ.value(element().toUpper());
}

QString Nuclide::element() const
{
    return el;
}

QString Nuclide::name() const
{
    return el + "-" + QString::number(m_A);
}

void Nuclide::addLevels(const QMap<double, EnergyLevel *> &levels)
{
    m_levels = levels;
}

QMap<double, EnergyLevel *> Nuclide::levels() const
{
    return m_levels;
}

QString Nuclide::halfLifeAsText() const
{
    QStringList result;
    foreach (HalfLife h, hl)
        if (h.isValid() && !h.isStable())
        result.append(h.toString());
    return result.join(", ");
}

QGraphicsItem *Nuclide::createNuclideGraphicsItem(const QFont &nucFont, const QFont &nucIndexFont)
{
    static const double numberToNameDistance = 4.0;

    QFontMetrics nucFontMetrics(nucFont);
    QFontMetrics nucIndexFontMetrics(nucIndexFont);

    item = new QGraphicsItemGroup;

    QGraphicsSimpleTextItem *granuc = new QGraphicsSimpleTextItem(element(), item);
    granuc->setFont(nucFont);
    granuc->setBrush(QBrush(QColor(64, 166, 255)));

    QGraphicsSimpleTextItem *graA = new QGraphicsSimpleTextItem(QString::number(a()), item);
    graA->setFont(nucIndexFont);
    graA->setBrush(QBrush(QColor(64, 166, 255)));

    QGraphicsSimpleTextItem *graZ = new QGraphicsSimpleTextItem(QString::number(z()), item);
    graZ->setFont(nucIndexFont);
    graZ->setBrush(QBrush(QColor(64, 166, 255)));

    double numberwidth = qMax(graA->boundingRect().width(), graZ->boundingRect().width());

    granuc->setPos(numberwidth + numberToNameDistance, 0.2*nucIndexFontMetrics.ascent());
    graA->setPos(numberwidth - graA->boundingRect().width(), 0.0);
    graZ->setPos(numberwidth - graZ->boundingRect().width(), 1.2*nucIndexFontMetrics.ascent());

    // added in the end to work around a bug in QGraphicsItemGroup:
    //   it does not update boundingRect if contents are moved after adding them
    item->addToGroup(granuc);
    item->addToGroup(graA);
    item->addToGroup(graZ);

    return item;
}

QGraphicsItem *Nuclide::nuclideGraphicsItem() const
{
    return item;
}

const QMap<QString, unsigned int> Nuclide::elToZ = initElToZ();

QMap<QString, unsigned int> Nuclide::initElToZ()
{
    QMap<QString, unsigned int> result;

    result["H"] = 1;
    result["HE"] = 2;
    result["LI"] = 3;
    result["BE"] = 4;
    result["B"] = 5;
    result["C"] = 6;
    result["N"] = 7;
    result["O"] = 8;
    result["F"] = 9;
    result["NE"] = 10;
    result["NA"] = 11;
    result["MG"] = 12;
    result["AL"] = 13;
    result["SI"] = 14;
    result["P"] = 15;
    result["S"] = 16;
    result["CL"] = 17;
    result["AR"] = 18;
    result["K"] = 19;
    result["CA"] = 20;
    result["SC"] = 21;
    result["TI"] = 22;
    result["V"] = 23;
    result["CR"] = 24;
    result["MN"] = 25;
    result["FE"] = 26;
    result["CO"] = 27;
    result["NI"] = 28;
    result["CU"] = 29;
    result["ZN"] = 30;
    result["GA"] = 31;
    result["GE"] = 32;
    result["AS"] = 33;
    result["SE"] = 34;
    result["BR"] = 35;
    result["KR"] = 36;
    result["RB"] = 37;
    result["SR"] = 38;
    result["Y"] = 39;
    result["ZR"] = 40;
    result["NB"] = 41;
    result["MO"] = 42;
    result["TC"] = 43;
    result["RU"] = 44;
    result["RH"] = 45;
    result["PD"] = 46;
    result["AG"] = 47;
    result["CD"] = 48;
    result["IN"] = 49;
    result["SN"] = 50;
    result["SB"] = 51;
    result["TE"] = 52;
    result["I"] = 53;
    result["XE"] = 54;
    result["CS"] = 55;
    result["BA"] = 56;
    result["LA"] = 57;
    result["CE"] = 58;
    result["PR"] = 59;
    result["ND"] = 60;
    result["PM"] = 61;
    result["SM"] = 62;
    result["EU"] = 63;
    result["GD"] = 64;
    result["TB"] = 65;
    result["DY"] = 66;
    result["HO"] = 67;
    result["ER"] = 68;
    result["TM"] = 69;
    result["YB"] = 70;
    result["LU"] = 71;
    result["HF"] = 72;
    result["TA"] = 73;
    result["W"] = 74;
    result["RE"] = 75;
    result["OS"] = 76;
    result["IR"] = 77;
    result["PT"] = 78;
    result["AU"] = 79;
    result["HG"] = 80;
    result["TL"] = 81;
    result["PB"] = 82;
    result["BI"] = 83;
    result["PO"] = 84;
    result["AT"] = 85;
    result["RN"] = 86;
    result["FR"] = 87;
    result["RA"] = 88;
    result["AC"] = 89;
    result["TH"] = 90;
    result["PA"] = 91;
    result["U"] = 92;
    result["NP"] = 93;
    result["PU"] = 94;
    result["AM"] = 95;
    result["CM"] = 96;
    result["BK"] = 97;
    result["CF"] = 98;
    result["ES"] = 99;
    result["FM"] = 100;
    result["MD"] = 101;
    result["NO"] = 102;
    result["LR"] = 103;
    result["RF"] = 104;
    result["DB"] = 105;
    result["SG"] = 106;
    result["BH"] = 107;
    result["HS"] = 108;
    result["MT"] = 109;
    result["DS"] = 110;
    result["RG"] = 111;
    result["CN"] = 112;
    result["UUT"] = 113;
    result["FL"] = 114;
    result["LV"] = 116;

    return result;
}
