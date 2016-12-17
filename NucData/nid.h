#ifndef NUCLIDE_ID_H
#define NUCLIDE_ID_H

#include <map>
#include <string>

class QGraphicsItem;
class QGraphicsItemGroup;
class EnergyLevel;

class NuclideId
{
public:
  NuclideId() : Z_(0), N_(0)  {}

  static NuclideId fromAZ(uint16_t a, uint16_t z);
  static NuclideId fromZN(uint16_t z, uint16_t n);

  static int16_t zOfSymbol(std::string name);
  static std::string nameOf(uint16_t Z);
  static std::string symbolOf(uint16_t Z);

  inline bool valid() const { return 0 != A(); }

  inline uint16_t A() const { return N_ + Z_; }
  inline uint16_t N() const { return N_; }
  inline uint16_t Z() const { return Z_; }
  void set_A(uint16_t a);
  void set_N(uint16_t n);
  void set_Z(uint16_t z);

  friend bool operator!=(const NuclideId &left, const NuclideId &right);
  friend bool operator==(const NuclideId &left, const NuclideId &right);
  friend bool operator<(const NuclideId &left, const NuclideId &right);
  friend bool operator>(const NuclideId &left, const NuclideId &right);

  std::string element() const;
  std::string symbolicName() const;
  std::string verboseName() const;

private:
  int16_t Z_;   // number of protons
  int16_t N_;   // number of neutrons

  struct NuclideNomenclature
  {
    NuclideNomenclature() {}
    NuclideNomenclature(std::string s, std::string n) { symbol=s; name=n; }
    std::string symbol;
    std::string name;
  };
  static const std::map<uint16_t, NuclideNomenclature> names;
  static std::map<uint16_t, NuclideNomenclature> initNames();
};


#endif
