#pragma once

#include <map>
#include <string>

class NuclideId
{
public:
  NuclideId() {}

  static NuclideId fromAZ(uint16_t a, uint16_t z, bool mass_only = false);
  static NuclideId fromZN(uint16_t z, uint16_t n, bool mass_only = false);

  static int16_t zOfSymbol(std::string name);
  static std::string nameOf(uint16_t Z);
  static std::string symbolOf(uint16_t Z);

  bool valid() const;

  uint16_t A() const;
  uint16_t N() const;
  uint16_t Z() const;
  bool composition_known() const;
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
  int16_t Z_ {0};   // number of protons
  int16_t N_ {0};   // number of neutrons
  bool mass_only_ {false};

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
