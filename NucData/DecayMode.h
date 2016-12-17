#ifndef DECAY_MODE_H
#define DECAY_MODE_H

#include <set>
#include <string>

class DecayMode
{
public:
  bool valid() const;

  std::string to_string() const;

  void set_alpha(bool);
  void set_isomeric(bool);
  void set_spontaneous_fission(bool);
  void set_protons(uint16_t);
  void set_neutrons(uint16_t);
  void set_beta_plus(uint16_t);
  void set_beta_minus(uint16_t);
  void set_electron_capture(uint16_t);

  bool alpha() const;
  bool isomeric() const;
  bool spontaneous_fission() const;
  uint16_t protons() const;
  uint16_t neutrons() const;
  uint16_t beta_plus() const;
  uint16_t beta_minus() const;
  uint16_t electron_capture() const;

private:
  uint16_t protons_ {0};
  uint16_t neutrons_ {0};

  uint16_t beta_plus_ {0};
  uint16_t beta_minus_ {0};
  uint16_t electron_capture_ {0};

  bool isomeric_ {false};
  bool alpha_ {false};
  bool spontaneous_fission_{false};

};

#endif
