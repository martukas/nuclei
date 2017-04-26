#include "nid.h"
#include "qpx_util.h"

bool NuclideId::valid() const
{
  return 0 != A();
}

uint16_t NuclideId::A() const
{
  return N_ + Z_;
}

uint16_t NuclideId::N() const
{
  return N_;
}

uint16_t NuclideId::Z() const
{
  return Z_;
}

bool NuclideId::composition_known() const
{
  return !mass_only_;
}

void NuclideId::set_A(uint16_t a)
{
  // keeps Z the same;
  if (a < Z_)
    a = Z_;
  N_ = a - Z_;
}

void NuclideId::set_N(uint16_t n)
{
  N_ = n;
}

void NuclideId::set_Z(uint16_t z)
{
  Z_ = z;
}


NuclideId NuclideId::fromAZ(uint16_t a, uint16_t z, bool mass_only)
{
  if (a < z)
    return NuclideId();
  NuclideId ret;
  ret.Z_ = z;
  ret.N_ = a - z;
  ret.mass_only_ = mass_only;
  return ret;
}

NuclideId NuclideId::fromZN(uint16_t z, uint16_t n, bool mass_only)
{
  NuclideId ret;
  ret.Z_ = z;
  ret.N_ = n;
  ret.mass_only_ = mass_only;
  return ret;
}

std::string NuclideId::symbolicName() const
{
  if (!valid())
    return "";
  if (mass_only_)
    return std::to_string(A());
  return symbolOf(Z_) + "-" + std::to_string(A());
}

std::string NuclideId::verboseName() const
{
  if (!valid())
    return "";
  if (mass_only_)
    return std::to_string(A());
  return nameOf(Z_) + "-" + std::to_string(A());
}


std::string NuclideId::element() const
{
  return symbolOf(Z_);
}

std::string NuclideId::symbolOf(uint16_t Z)
{
  if (names.count(Z))
    return names.at(Z).symbol;
  else
    return "";
}


std::string NuclideId::nameOf(uint16_t Z)
{
  if (names.count(Z))
    return names.at(Z).name;
  else
    return "";
}

int16_t NuclideId::zOfSymbol(std::string name)
{
  for (auto &nom : names) {
    if ((boost::to_upper_copy(nom.second.symbol) == name)
        || (boost::to_upper_copy(nom.second.name) == name)) {
      return nom.first;
    }
  }
  return -1;
}

bool operator!=(const NuclideId &left, const NuclideId &right)
{
  return !(left == right);
}

bool operator==(const NuclideId &left, const NuclideId &right)
{
  return ((left.N_==right.N_) && (left.Z_ == right.Z_));
}

bool operator<(const NuclideId &left, const NuclideId &right)
{
  if (left.A() == right.A())
    return (left.N_ < right.N_);
  else
    return (left.A() < right.A());
}

bool operator>(const NuclideId &left, const NuclideId &right)
{
  if (left.A() == right.A())
    return (left.N_ > right.N_);
  else
    return (left.A() > right.A());
}


const std::map<uint16_t, NuclideId::NuclideNomenclature> NuclideId::names = initNames();

std::map<uint16_t, NuclideId::NuclideNomenclature> NuclideId::initNames()
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
  result[112] = NuclideNomenclature("Cn", "Copernicium");
  result[113] = NuclideNomenclature("Nh", "Nihonium");    //Nh
  result[114] = NuclideNomenclature("Fl", "Flerovium");   //Fl
  result[115] = NuclideNomenclature("Mc", "Moscovium");   //Mc
  result[116] = NuclideNomenclature("Lv", "Livermorium"); //Lv
  result[117] = NuclideNomenclature("Ts", "Tennessine");  //Ts
  result[118] = NuclideNomenclature("Og", "Oganesson");   //Og
  return result;
}
