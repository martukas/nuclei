#include "Nuclide.h"
#include <QGraphicsItemGroup>
#include <QGraphicsSimpleTextItem>
#include <QFontMetrics>
#include <QBrush>
#include <boost/algorithm/string.hpp>
#include "custom_logger.h"

Nuclide::Nuclide()
  : m_A(0), item(0)
{
}

Nuclide::Nuclide(uint16_t A, uint16_t Z, const HalfLife &halfLife)
  : m_A(A), m_Z(Z), item(0)
{
  hl.append(halfLife);
}

Nuclide::Nuclide(uint16_t A, uint16_t Z, const QList<HalfLife> &halfLifes)
  : m_A(A), m_Z(Z), hl(halfLifes), item(0)
{
}

uint16_t Nuclide::a() const
{
  return m_A;
}

uint16_t Nuclide::z() const
{
  return m_Z;
}

Nuclide::Coordinates Nuclide::coordinates() const
{
  return Coordinates(a(), z());
}

QString Nuclide::symbolicName(Nuclide::Coordinates c)
{
  if ((c.first <= 0) || (c.second < 0)) {
    DBG << "empty " << c.first << " " << c.second;
    return "";
  }
  return symbolOf(c.second) + "-" + QString::number(c.first);
}

QString Nuclide::symbolOf(uint16_t Z)
{
  if (names.count(Z))
    return QString::fromStdString(names.at(Z).symbol);
  else
    return "";
}

int16_t Nuclide::zOfSymbol(const QString &name)
{
  std::string searchname = name.toStdString();
  boost::to_upper(searchname);

  for (auto &nom : names) {
    if ((boost::to_upper_copy(nom.second.symbol) == searchname)
        || (boost::to_upper_copy(nom.second.name) == searchname)) {
      return nom.first;
    }
  }
//  DBG << "not found " << searchname;
  return -1;
}

QString Nuclide::element() const
{
  return symbolOf(m_Z);
}

QString Nuclide::name() const
{
  return element() + "-" + QString::number(m_A);
}

void Nuclide::addLevels(const QMap<Energy, EnergyLevel *> &levels)
{
  m_levels.unite(levels);
}

QMap<Energy, EnergyLevel *> &Nuclide::levels()
{
  return m_levels;
}

QList<HalfLife> Nuclide::halfLifes() const
{
  return hl;
}

QString Nuclide::halfLifeAsText() const
{
  QStringList result;
  foreach (HalfLife h, hl)
    if (h.isValid() && !h.isStable())
      result.append(QString::fromStdString(h.to_string()));
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

//const QMap<QString, uint16_t> Nuclide::elToZ = initElToZ();

const std::map<uint16_t, NuclideNomenclature> Nuclide::names = initNames();

std::map<uint16_t, NuclideNomenclature> Nuclide::initNames()
{
  std::map<uint16_t, NuclideNomenclature> result;
//  result[0]   = NuclideNomenclature("n", "neutron");

  result[0]   = NuclideNomenclature("nn", "neutron");
  result[1]   = NuclideNomenclature("H",  "Hydrogen");
  result[2]   = NuclideNomenclature("He", "Helium");
  result[3]   = NuclideNomenclature("Li", "Lithium");
  result[4]   = NuclideNomenclature("Be", "Beryllium");
  result[5]   = NuclideNomenclature("B",  "Boron");
  result[6]   = NuclideNomenclature("C",  "Carbon");
  result[7]   = NuclideNomenclature("N",  "Nitrogen");
  result[8]   = NuclideNomenclature("O",  "Oxygen");
  result[9]   = NuclideNomenclature("F",  "Fluorine");
  result[10]  = NuclideNomenclature("Ne", "Neon");
  result[11]  = NuclideNomenclature("Na", "Sodium");
  result[12]  = NuclideNomenclature("Mg", "Magnesium");
  result[13]  = NuclideNomenclature("Al", "Aluminium");
  result[14]  = NuclideNomenclature("Si", "Silicon");
  result[15]  = NuclideNomenclature("P",  "Phosphorus");
  result[16]  = NuclideNomenclature("S",  "Sulfur");
  result[17]  = NuclideNomenclature("Cl", "Chlorine");
  result[18]  = NuclideNomenclature("Ar", "Argon");
  result[19]  = NuclideNomenclature("K",  "Potassium");
  result[20]  = NuclideNomenclature("Ca", "Calcium");
  result[21]  = NuclideNomenclature("Sc", "Scandium");
  result[22]  = NuclideNomenclature("Ti", "Titanium");
  result[23]  = NuclideNomenclature("V",  "Vanadium");
  result[24]  = NuclideNomenclature("Cr", "Chromium");
  result[25]  = NuclideNomenclature("Mn", "Manganese");
  result[26]  = NuclideNomenclature("Fe", "Iron");
  result[27]  = NuclideNomenclature("Co", "Cobalt");
  result[28]  = NuclideNomenclature("Ni", "Nickel");
  result[29]  = NuclideNomenclature("Cu", "Copper");
  result[30]  = NuclideNomenclature("Zn", "Zinc");
  result[31]  = NuclideNomenclature("Ga", "Gallium");
  result[32]  = NuclideNomenclature("Ge", "Germanium");
  result[33]  = NuclideNomenclature("As", "Arsenic");
  result[34]  = NuclideNomenclature("Se", "Selenium");
  result[35]  = NuclideNomenclature("Br", "Bromine");
  result[36]  = NuclideNomenclature("Kr", "Krypton");
  result[37]  = NuclideNomenclature("Rb", "Rubidium");
  result[38]  = NuclideNomenclature("Sr", "Strontium");
  result[39]  = NuclideNomenclature("Y",  "Yttrium");
  result[40]  = NuclideNomenclature("Zr", "Zirconium");
  result[41]  = NuclideNomenclature("Nb", "Niobium");
  result[42]  = NuclideNomenclature("Mo", "Molybdenum");
  result[43]  = NuclideNomenclature("Tc", "Technetium");
  result[44]  = NuclideNomenclature("Ru", "Ruthenium");
  result[45]  = NuclideNomenclature("Rh", "Rhodium");
  result[46]  = NuclideNomenclature("Pd", "Palladium");
  result[47]  = NuclideNomenclature("Ag", "Silver");
  result[48]  = NuclideNomenclature("Cd", "Cadmium");
  result[49]  = NuclideNomenclature("In", "Indium");
  result[50]  = NuclideNomenclature("Sn", "Tin");
  result[51]  = NuclideNomenclature("Sb", "Antimony");
  result[52]  = NuclideNomenclature("Te", "Tellurium");
  result[53]  = NuclideNomenclature("I",  "Iodine");
  result[54]  = NuclideNomenclature("Xe", "Xenon");
  result[55]  = NuclideNomenclature("Cs", "Caesium");
  result[56]  = NuclideNomenclature("Ba", "Barium");
  result[57]  = NuclideNomenclature("La", "Lanthanum");
  result[58]  = NuclideNomenclature("Ce", "Cerium");
  result[59]  = NuclideNomenclature("Pr", "Praseodynium");
  result[60]  = NuclideNomenclature("Nd", "Neodynium");
  result[61]  = NuclideNomenclature("Pm", "Promethium");
  result[62]  = NuclideNomenclature("Sm", "Samarium");
  result[63]  = NuclideNomenclature("Eu", "Europium");
  result[64]  = NuclideNomenclature("Gd", "Gadolinium");
  result[65]  = NuclideNomenclature("Tb", "Terbium");
  result[66]  = NuclideNomenclature("Dy", "Dysprosium");
  result[67]  = NuclideNomenclature("Ho", "Holmium");
  result[68]  = NuclideNomenclature("Er", "Erbium");
  result[69]  = NuclideNomenclature("Tm", "Thulium");
  result[70]  = NuclideNomenclature("Yb", "Ytterbium");
  result[71]  = NuclideNomenclature("Lu", "Lutetium");
  result[72]  = NuclideNomenclature("Hf", "Hafnium");
  result[73]  = NuclideNomenclature("Ta", "Tantalum");
  result[74]  = NuclideNomenclature("W",  "Tungsten");
  result[75]  = NuclideNomenclature("Re", "Rhenium");
  result[76]  = NuclideNomenclature("Os", "Osmium");
  result[77]  = NuclideNomenclature("Ir", "Iridium");
  result[78]  = NuclideNomenclature("Pt", "Platinum");
  result[79]  = NuclideNomenclature("Au", "Gold");
  result[80]  = NuclideNomenclature("Hg", "Mercury");
  result[81]  = NuclideNomenclature("Tl", "Thallium");
  result[82]  = NuclideNomenclature("Pb", "Lead");
  result[83]  = NuclideNomenclature("Bi", "Bismuth");
  result[84]  = NuclideNomenclature("Po", "Polonium");
  result[85]  = NuclideNomenclature("At", "Astanine");
  result[86]  = NuclideNomenclature("Rn", "Radon");
  result[87]  = NuclideNomenclature("Fr", "Francium");
  result[88]  = NuclideNomenclature("Ra", "Radium");
  result[89]  = NuclideNomenclature("Ac", "Actinium");
  result[90]  = NuclideNomenclature("Th", "Thorium");
  result[91]  = NuclideNomenclature("Pa", "Protactinium");
  result[92]  = NuclideNomenclature("U",  "Uranium");
  result[93]  = NuclideNomenclature("Np", "Neptunium");
  result[94]  = NuclideNomenclature("Pu", "Plutonium");
  result[95]  = NuclideNomenclature("Am", "Americium");
  result[96]  = NuclideNomenclature("Cm", "Curium");
  result[97]  = NuclideNomenclature("Bk", "Berkelium");
  result[98]  = NuclideNomenclature("Cf", "Californium");
  result[99]  = NuclideNomenclature("Es", "Einsteinium");
  result[100] = NuclideNomenclature("Fm", "Fermium");
  result[101] = NuclideNomenclature("Md", "Mendelevium");
  result[102] = NuclideNomenclature("No", "Nobelium");
  result[103] = NuclideNomenclature("Lr", "Lawrencium");
  result[104] = NuclideNomenclature("Rf", "Rutherfordium");
  result[105] = NuclideNomenclature("Db", "Dubnium");
  result[106] = NuclideNomenclature("Sg", "Seaborgium");
  result[107] = NuclideNomenclature("Bh", "Bohrium");
  result[108] = NuclideNomenclature("Hs", "Hassium");
  result[109] = NuclideNomenclature("Mt", "Meitnerium");
  result[110] = NuclideNomenclature("Ds", "Darmstadtium");
  result[111] = NuclideNomenclature("Rg", "Roentgenium");
  result[112] = NuclideNomenclature("Cn",  "Copernicium");
  result[113] = NuclideNomenclature("Uut", "Ununtrium");
  result[114] = NuclideNomenclature("Fl",  "Flerovium");
  result[115] = NuclideNomenclature("Uup", "Ununpentium");
  result[116] = NuclideNomenclature("Lv",  "Livermorium");
  result[117] = NuclideNomenclature("Uus", "Ununseptium");
  result[118] = NuclideNomenclature("Uuo", "Ununoctium");
  return result;
}
