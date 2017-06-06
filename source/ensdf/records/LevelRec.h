#pragma once

#include "Alpha.h"
#include "Beta.h"
#include "Gamma.h"
#include "EC.h"
#include "Particle.h"

#include "qpx_util.h"

struct Transitions
{
  std::list<AlphaRecord> alpha;
  std::list<BetaRecord> beta;
  std::list<GammaRecord> gamma;
  std::list<ECRecord> EC;
  std::list<ParticleRecord> particle;
};

struct LevelRecord
{
  LevelRecord() {}
  LevelRecord(ENSDFData& i);
  static bool match(const std::string& line);
  void merge_adopted(const LevelRecord& other,
                     double max_gamma_dif = 0.005);

  std::string debug() const;
  bool valid() const;

  NuclideId  nuclide;
  Energy     energy;
  SpinSet    spins;
  HalfLife   halflife;
  uint16_t   isomeric {0};
  std::string L;     //anglular momentum
  Uncert S; //spectroscopic strength
  std::string comment_flag, quality;

  std::map<std::string, Continuation> continuations_;

  std::list<std::string> offsets;

  std::list<CommentsRecord> comments;

  Transitions transitions;

  std::list<GammaRecord> nearest_gammas(const Energy& to,
                                        double maxdif = kDoubleNaN) const;

private:
  void parse_energy_offset(std::string val,
                           std::string uncert);

  std::string offsets_to_str() const;
};
