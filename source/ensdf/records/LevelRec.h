#pragma once

#include "Alpha.h"
#include "Beta.h"
#include "Gamma.h"
#include "EC.h"
#include "Particle.h"

#include "qpx_util.h"

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
  SpinParity spin_parity;
  HalfLife   halflife;
  uint16_t   isomeric {0};
  std::string L;     //anglular momentum
  Uncert S; //spectroscopic strength
  std::string comment_flag, quality;

  std::map<std::string, std::string> continuations_;

  std::list<std::string> offsets;

  std::list<CommentsRecord> comments;

  std::list<AlphaRecord> alphas;
  std::list<BetaRecord> betas;
  std::list<GammaRecord> gammas;
  std::list<ECRecord> ECs;
  std::list<ParticleRecord> particles;

  std::list<GammaRecord> nearest_gammas(const Energy& to,
                                        double maxdif = kDoubleNaN) const;

private:
  void parse_energy_offset(std::string val,
                           std::string uncert);
};
