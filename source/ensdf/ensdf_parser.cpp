#include "ensdf_parser.h"
#include "ensdf_types.h"

#include "custom_logger.h"
#include "qpx_util.h"

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>


ENSDFParser::ENSDFParser()
{}

ENSDFParser::ENSDFParser(std::string directory)
{
  if (directory.empty())
    return;

  boost::system::error_code c;
  boost::filesystem::path path(directory);
  bool is_dir = boost::filesystem::is_directory(path, c);

  if (!is_dir)
    return;

  directory_ = directory;

  boost::filesystem::directory_iterator end_iter;

  for( boost::filesystem::directory_iterator dir_iter(path);
       dir_iter != end_iter ; ++dir_iter)
    if (boost::filesystem::is_regular_file(dir_iter->status()) &&
        (dir_iter->path().string().size() > 3))
    {
      std::string filename = dir_iter->path().string();
      std::string a_num = filename.substr(filename.size()-3, 3);
      if (is_number(a_num))
        masses_.insert(boost::lexical_cast<uint16_t>(a_num));
    }
}

bool ENSDFParser::good() const
{
  return !masses_.empty();
}

std::list<uint16_t> ENSDFParser::masses() const
{
  return std::list<uint16_t>(masses_.begin(), masses_.end());
}

std::string ENSDFParser::directory() const
{
  return directory_;
}

DaughterParser ENSDFParser::get_dp(uint16_t a)
{
  if (!masses_.count(a))
    return DaughterParser();
  else if (!cache_.count(a))
    cache_[a] = DaughterParser(a, directory_);
  return cache_[a];
}




DaughterParser::DaughterParser()
{

}

DaughterParser::DaughterParser(uint16_t A, std::string directory)
{
  std::string num = std::to_string(A);
  if (num.size() < 3)
    num = std::string(3-num.size(), '0') + num;
  std::string file = directory + "/ensdf." + num;

  std::ifstream t(file);
  std::string str;

  t.seekg(0, std::ios::end);
  str.reserve(t.tellg());
  t.seekg(0, std::ios::beg);

  str.assign((std::istreambuf_iterator<char>(t)),
             std::istreambuf_iterator<char>());

  boost::split(raw_contents_, str, boost::is_any_of("\n"));

  parseBlocks();
  mass_num_ = A;
}

uint16_t DaughterParser::mass_num() const
{
  return mass_num_;
}

std::list<NuclideId> DaughterParser::daughters() const
{
  //  Q_ASSERT(decays_.size() == decays_.keys().toSet().size()); // check for duplicates
  std::list<NuclideId> ret;
  for (auto &n : decays_)
    ret.push_back(n.first);
  return ret;
}

std::list<std::pair<std::string, NuclideId> > DaughterParser::decays(NuclideId daughter) const
{
  //  Q_ASSERT(decays_.value(daughterNuclide).size() == decays_.value(daughterNuclide).keys().toSet().size()); // check for duplicates
  std::list< std::pair<std::string, NuclideId> > result;
  for (auto &i : decays_.at(daughter))
    result.push_back(std::pair<std::string, NuclideId>(i.first, i.second.parents.at(0).nuclide));
  return result;
}

DecayScheme DaughterParser::get_decay(NuclideId daughter,
                                  std::string decay_name) const
{
  if (!adopted_levels_.count(daughter) ||
      !decays_.count(daughter) ||
      !decays_.at(daughter).count(decay_name))
    return DecayScheme();

  //  QSettings s;
  //  double adoptedLevelMaxDifference = s.value("preferences/levelTolerance", 1.0).toDouble();
  //  double gammaMaxDifference = s.value("preferences/gammaTolerance", 1.0).toDouble();

  double adoptedLevelMaxDifference = 40.0;
  double gammaMaxDifference = 5.0;

  BlockIndices alpos = adopted_levels_.at(daughter);
  BasicDecayData decaydata = decays_.at(daughter).at(decay_name);

  Nuclide daughter_nuclide(decaydata.daughter);

  double normalizeDecIntensToPercentParentDecay = 1.0;
  double normalizeGammaIntensToPercentParentDecay = 1.0;
  std::string dNucid1 = nid_to_ensdf(daughter, true);
  std::string dNucid2 = nid_to_ensdf(daughter, false);

  //  DBG << "parsing " << dNucid.toStdString();

  // process all adopted level sub-blocks
  Level currentLevel;
  // create index map for adopted levels
  std::map<Energy, BlockIndices> adoptblocks;
  std::map<std::string, char> xrefs; // maps DSID to DSSYM (single letter)
  int laststart = -1;
  for (size_t i=alpos.first; i < alpos.last; i++)
  {
    const std::string line = raw_contents_.at(i);
    // extract cross reference records as long as first level was found (cross reference must be before 1st level)
    if (laststart == -1) {
      if (line.substr(0,8) == (dNucid1 + "  X"))
        xrefs[boost::trim_copy(line.substr(9, 30))] = line[8];
    }
    // find level records
    if (line.substr(0,9) == (dNucid1 + "  L ")) {
      if (laststart > 0) {
        size_t i1 = laststart;
        size_t i2 = i;
        BlockIndices sl(i1, i2);
        if ((sl.first < sl.last) &&
            (sl.last <= raw_contents_.size()) &&
            xrefs.count(decaydata.dsid))
          insertAdoptedLevelsBlock(&adoptblocks, sl, xrefs.at(decaydata.dsid));
      }
      laststart = i;
    }
  }
  if (laststart > 0)
  {
    BlockIndices sl(laststart, alpos.last);
    if ((sl.first < sl.last) &&
        (sl.last <= raw_contents_.size()) &&
        xrefs.count(decaydata.dsid))
      insertAdoptedLevelsBlock(&adoptblocks, sl, xrefs.at(decaydata.dsid));
  }

  laststart = -1;
  for (size_t i=alpos.first; i < alpos.last; i++)
  {
    const std::string line = raw_contents_.at(i);
    // extract cross reference records as long as first level was found (cross reference must be before 1st level)
    if (laststart == -1) {
      if (line.substr(0,8) == (dNucid2 + "  X"))
        xrefs[boost::trim_copy(line.substr(9, 30))] = line[8];
    }
    // find level records
    if (line.substr(0,9) == (dNucid2 + "  L ")) {
      if (laststart > 0) {
        size_t i1 = laststart;
        size_t i2 = i;
        BlockIndices sl(i1, i2);
        if ((sl.first < sl.last) &&
            (sl.last <= raw_contents_.size()) &&
            xrefs.count(decaydata.dsid))
          insertAdoptedLevelsBlock(&adoptblocks, sl, xrefs.at(decaydata.dsid));
      }
      laststart = i;
    }
  }
  if (laststart > 0)
  {
    BlockIndices sl(laststart, alpos.last);
    if ((sl.first < sl.last) &&
        (sl.last <= raw_contents_.size()) &&
        xrefs.count(decaydata.dsid))
      insertAdoptedLevelsBlock(&adoptblocks, sl, xrefs.at(decaydata.dsid));
  }


  // adopted levels block of current level in decay data set
  BlockIndices currentadoptblock;

  // process decay block

  //  DBG << "parsing size " << decaylines.size();

  for (size_t k=decaydata.block.first; k < decaydata.block.last; k++)
  {
    const std::string line = raw_contents_.at(k);

    //    DBG << "c line " << line.toStdString();

    // process new gamma
    if ((line.size() >= 8) &&
        ((line.substr(0,9) == (dNucid1 + "  G ")) ||
         (line.substr(0,9) == (dNucid2 + "  G "))))
    {

      // determine energy
      Energy e = parse_energy(line.substr(9, 12));

      // determine intensity
      std::string instr = line.substr(21,8);
      boost::replace_all(instr, "(", "");
      boost::replace_all(instr, ")", "");
      boost::trim(instr);
      double  in = std::numeric_limits<double>::quiet_NaN();
      if (is_number(instr))
        in = boost::lexical_cast<double>(instr) * normalizeGammaIntensToPercentParentDecay;

      // determine multipolarity
      std::string mpol = boost::trim_copy(line.substr(31, 10));

      // determine delta
      UncertainDouble delta = parseEnsdfMixing(line.substr(41, 14), mpol);

      // parse adopted levels if necessary
      if ((delta.sign() != UncertainDouble::SignMagnitudeDefined) || mpol.empty()) {
        // create gamma map
        std::map<Energy, std::string> e2g;
        for (size_t i = currentadoptblock.first; i < currentadoptblock.last; ++i)
        {
          std::string ln = raw_contents_.at(i);
          if ((ln.substr(0,9) ==  (dNucid1 + "  G ")) || (ln.substr(0,9) ==  (dNucid2 + "  G ")))
          {
            Energy gk = parse_energy(ln.substr(9, 12));
            if (gk.valid())
              e2g[gk] = ln;
          }
        }
        // find gamma
        if (!e2g.empty()) {
          Energy foundE;
          const std::string gammastr = e2g.at(findNearest(e2g, e, &foundE));
          if (e-foundE < gammaMaxDifference/1000.0*e) {
            if (mpol.empty())
              mpol = boost::trim_copy(gammastr.substr(31, 10));

            if (delta.sign() != UncertainDouble::SignMagnitudeDefined) {
              UncertainDouble adptdelta = parseEnsdfMixing(gammastr.substr(41, 14), mpol);
              if (adptdelta.sign() > delta.sign())
                delta = adptdelta;
            }
          }
        }
      }

      // determine levels
      if (!daughter_nuclide.empty())
      {
        Energy start = currentLevel.energy();
        Energy destlvl = findNearest(daughter_nuclide.levels(), start - e);
        daughter_nuclide.addTransition(Transition(e, in, mpol, delta, start, destlvl));
      }
    }
    // process new level
    else if ((line.substr(0,9) == (dNucid1 + "  L ")) ||
             (line.substr(0,9) == (dNucid2 + "  L ")))
    {

      currentLevel = parse_level(line);

      // get additional data from adopted leves record
      //   find closest entry
      if (!adoptblocks.empty()) {
        Energy foundE;
        currentadoptblock.clear();
        currentadoptblock = adoptblocks.at(findNearest(adoptblocks, currentLevel.energy(), &foundE));
        //        Q_ASSERT(currentadoptblock.last >= currentadoptblock.first);
        if (abs((currentLevel.energy() - foundE).operator double()) > (adoptedLevelMaxDifference/1000.0*currentLevel.energy()))
          currentadoptblock.clear();
      }
      else
        currentadoptblock.clear();

      // if an appropriate entry was found, read its raw_contents_
      // set half life if necessary
      if (currentadoptblock.first != currentadoptblock.last)
      {
        std::string levelfirstline(raw_contents_.at(currentadoptblock.first));
//                DBG << levelfirstline << " === " << currentLevel->to_string();
        if (!currentLevel.halfLife().isValid()) {
          currentLevel.set_halflife(parse_halflife(levelfirstline.substr(39, 16)));
          if (!currentLevel.spin().valid())
            currentLevel.set_spin(parse_spin_parity(levelfirstline.substr(21, 18)));
//          DBG << "newhl" << currentLevel->halfLife().to_string();
        }
        // parse continuation records
        std::list<std::string> momentsRequestList;
        momentsRequestList.push_back("MOME2");
        momentsRequestList.push_back("MOMM1");
        std::vector<std::string> moms = extractContinuationRecords(currentadoptblock, momentsRequestList);
        currentLevel.set_q(parse_moment(moms.at(0)));
        currentLevel.set_mu(parse_moment(moms.at(1)));
//                DBG << levelfirstline << " === " << currentLevel->to_string();
      }

      daughter_nuclide.addLevel(currentLevel);
    }
    // process decay information
    else if (!daughter_nuclide.empty() &&
             ((line.substr(0,9) == (dNucid1 + "  E ")) || (line.substr(0,9) == (dNucid2 + "  E ")))) {
      UncertainDouble ti = parse_val_uncert(line.substr(64, 10), line.substr(74, 2));
      if (ti.uncertaintyType() != UncertainDouble::UndefinedType) {
        ti.setSign(UncertainDouble::SignMagnitudeDefined);
      }
      else {
        UncertainDouble ib = parse_val_uncert(line.substr(21, 8), line.substr(29, 2));
        UncertainDouble ie = parse_val_uncert(line.substr(31, 8), line.substr(39, 2));
        ti = ib;
        if (ib.uncertaintyType() != UncertainDouble::UndefinedType && ie.uncertaintyType() != UncertainDouble::UndefinedType)
          ti += ie;
        else if (ie.uncertaintyType() != UncertainDouble::UndefinedType)
          ti = ie;
        else
          ti = UncertainDouble();
      }
      if (ti.uncertaintyType() != UncertainDouble::UndefinedType)
      {
        currentLevel.setFeedIntensity(ti);
        daughter_nuclide.addLevel(currentLevel);
      }
    }
    else if (!daughter_nuclide.empty() &&
             ((line.substr(0,9) == (dNucid1 + "  B ")) || (line.substr(0,9) == (dNucid2 + "  B ")))) {
      UncertainDouble ib = parse_val_uncert(line.substr(21, 8), line.substr(29, 2));
      if (ib.hasFiniteValue()) {
        ib.setSign(UncertainDouble::SignMagnitudeDefined);
        ib *= normalizeDecIntensToPercentParentDecay;
        currentLevel.setFeedIntensity(ib);
        daughter_nuclide.addLevel(currentLevel);
      }
    }
    else if (!daughter_nuclide.empty() &&
             ((line.substr(0,9) == (dNucid1 + "  A ")) || (line.substr(0,9) == (dNucid2 + "  A ")))) {
      UncertainDouble ia = parse_val_uncert(line.substr(21, 8), line.substr(29, 2));
      if (ia.hasFiniteValue()) {
        ia.setSign(UncertainDouble::SignMagnitudeDefined);
        ia *= normalizeDecIntensToPercentParentDecay;
        currentLevel.setFeedIntensity(ia);
        daughter_nuclide.addLevel(currentLevel);
      }
    }
    // process normalization records
    else if ((line.substr(0,9) == (dNucid1 + "  N ")) || (line.substr(0,9) == (dNucid2 + "  N "))) {
      double br = norm(line.substr(31, 8), 1.0);
      double nb = norm(line.substr(41, 8), 1.0);
      normalizeDecIntensToPercentParentDecay = nb * br;
      double nr = norm(line.substr(9, 10), 1.0);
      normalizeGammaIntensToPercentParentDecay = nr * br;
    }
    else if ((line.substr(0,9) == (dNucid1 + " PN ")) || (line.substr(0,9) == (dNucid2 + " PN "))) {
      normalizeDecIntensToPercentParentDecay = norm(line.substr(41, 8), normalizeDecIntensToPercentParentDecay);
      normalizeGammaIntensToPercentParentDecay = norm(line.substr(9, 10), normalizeGammaIntensToPercentParentDecay);
    }
  }

  // create relevant parent levels and collect parent half-lifes
  Nuclide parent_nuclide(decaydata.parents.at(0).nuclide);
  for (ParentRecord p : decaydata.parents)
  {
    parent_nuclide.addHalfLife(p.hl);

    Level plv(p.energy, p.spin, p.hl);
    plv.setFeedingLevel(true);
    parent_nuclide.addLevel(plv);
  }

  if (!parent_nuclide.empty() &&
      (parent_nuclide.levels().begin()->second.energy() > 0.0))
  {
    Level plv(Energy(0.0, UncertainDouble::SignMagnitudeDefined), SpinParity(), HalfLife());
    plv.setFeedingLevel(false);
    parent_nuclide.addLevel(plv);
  }

  parent_nuclide.finalize();
  daughter_nuclide.finalize();
  //HACK types
  return DecayScheme(decay_name, parent_nuclide, daughter_nuclide, decaydata.mode);
}

double DaughterParser::norm(std::string rec, double def_value)
{
  boost::replace_all(rec, "(", "");
  boost::replace_all(rec, ")", "");
  boost::trim(rec);
  if (is_number(rec))
    def_value = boost::lexical_cast<double>(rec);
  return def_value;
}

UncertainDouble DaughterParser::parseEnsdfMixing(const std::string &mstr, const std::string &multipolarity)
{
  if (mstr.size() != 14)
    return UncertainDouble();

  // special case for pure multipolarities
  if (boost::trim_copy(mstr).empty())
  {
    std::string tmp(multipolarity);
    boost::replace_all(tmp, "(", "");
    boost::replace_all(tmp, ")", "");
    if (tmp.size() == 2)
      return UncertainDouble(0.0, 1, UncertainDouble::SignMagnitudeDefined);
    else
      return UncertainDouble();
  }

  return parse_val_uncert(mstr.substr(0,8), mstr.substr(8,6));
}

/**
 * @brief DaughterParser::insertAdoptedLevelsBlock
 * @param adoptblocks map of adopted levels (target)
 * @param newblock block to add
 * @param dssym Symbol of the current decay. Used to filter levels and adjust energies according to XREF records
 */
void DaughterParser::insertAdoptedLevelsBlock(std::map<Energy, BlockIndices> *adoptblocks, const BlockIndices &newblock, char dssym) const
{
  if (!adoptblocks || (newblock.last <= newblock.first))
    return;

  // get xref record
  std::list<std::string> req;
  req.push_back("XREF");
  std::vector<std::string> xreflist = extractContinuationRecords(newblock, req);
  std::string xref = xreflist.at(0);

  // filter data sets

  // -(AB) case (do not add level if dssym is contained in the parentheses)
  if ((xref.substr(0,2) == "-(")
      && (xref[xref.size()-1] == ')')
      && boost::contains(xref, std::string(1,dssym)))
    return;

  // exit if xref is neither "+" (level valid for all datasets) nor -(...) not containing dssymb
  // nor contains dssym
  if (xref != "+"
      && (xref.substr(0,2) != "-(")
      && !boost::contains(xref,std::string(1,dssym)))
    return;

  // if this point is reached the level will be added in any case

  // read energy from level record
  Energy e = parse_energy(raw_contents_.at(newblock.first).substr(9, 12));

  // for the A(E1) case the energy must be modified
  boost::regex er(dssym + "\\(([.\\d]+)\\)");
  boost::smatch what;
  if (boost::regex_match(xref, what, er) && (what.size() > 1)) {
    Energy matchedE = parse_energy(what[1]);
    DBG << "ENERGY FROM REGEXP " << xref << " --> " << matchedE.to_string() << " valid " << matchedE.valid();

    //std::cerr << "Current xref: " << xref.toStdString() << " current dssymb: " << dssym << std::endl;
    //std::cerr << "Energy translation, old: " << e << " new: " << matchedE << std::endl;
    if (matchedE.valid())
      e = matchedE;
  }

  // add record
  (*adoptblocks)[e] = newblock;
}

/**
 * @brief DaughterParser::extractContinuationRecords
 * @param adoptedblock block to search continuation records in
 * @param requestedRecords list of requested records
 * @param typeOfContinuedRecord type of record (default: L(evel))
 * @return list of found records (same size as requestedRecords, empty strings if no record was found)
 */
std::vector<std::string> DaughterParser::extractContinuationRecords(const BlockIndices &adoptedblock,
                                                                 const std::list<std::string> &requestedRecords,
                                                                 char typeOfContinuedRecord) const
{
  // fetch records
  boost::regex crecre("^[A-Z0-9\\s]{5,5}[A-RT-Z0-9] " + std::string(1,typeOfContinuedRecord) + " (.*)$");
  std::vector<std::string> crecs;
  for (size_t i = adoptedblock.first; i < adoptedblock.last; ++i)
  {
    boost::smatch what;
    if (boost::regex_search(raw_contents_.at(i), what, crecre) && (what.size() > 1))
       {
      crecs.push_back(raw_contents_.at(i));
    }
  }
  std::vector<std::string> crecs2;
  // remove record id from beginning of string
  for (size_t i=0; i<crecs.size(); i++) {
    crecs[i].erase(0, 9);
    crecs2.push_back(crecs.at(i));
  }
  // join lines and then split records
  std::string tmp = join(crecs2, "$");
  boost::split(crecs2, tmp, boost::is_any_of("$"));
  for (size_t i=0; i<crecs2.size(); i++)
    crecs2[i] = boost::trim_copy(crecs2[i]);
  // search and parse requested fields
  std::vector<std::string> result;
  for ( auto &req : requestedRecords) {
    std::string rstr;
    for (size_t i=0; i<crecs2.size(); i++) {
      if ((crecs2.at(i).size() >= req.size()) && (crecs2.at(i).substr(0, req.size()) == req)) {
        rstr = boost::trim_copy(crecs2.at(i).substr(5, crecs2.at(i).size() - 5));
        break;
      }
    }
    if (!rstr.empty() && (rstr[0] == '='))
      rstr = rstr.substr(1, rstr.size()-1);
    result.push_back(rstr);
  }
  return result;
}

template <typename T>
Energy DaughterParser::findNearest(const std::map<Energy, T> &map, const Energy &val, Energy *foundVal) const
{
  if (map.empty())
    return Energy();

  typename std::map<Energy, T>::const_iterator low, prev;
  low = map.lower_bound(val);
  if (low == map.end())
    low--;
  else if (low != map.begin()) {
    prev = low;
    --prev;
    if (abs(val - prev->first) < abs(low->first - val))
      low = prev;
  }

  if (foundVal)
    (*foundVal) = low->first;

  return low->first;
}

IdentificationRecord DaughterParser::parse_header(size_t idx)
{
  if (idx >= raw_contents_.size())
    return IdentificationRecord();
  IdentificationRecord header = IdentificationRecord::parse(raw_contents_.at(idx));
  if (header.continued)
    header.merge_continued(parse_header(idx+1));

  return header;
}

void DaughterParser::parseBlocks()
{
  // create list of block boundaries
  // end index points behind last line of block!
  std::list<BlockIndices> blocks;
  size_t from = 0;
  boost::regex emptyline("^\\s*$");
  for (size_t i=0; i < raw_contents_.size(); ++i)
    if (boost::regex_match(raw_contents_.at(i), emptyline))
    {
      if (i-from > 1)
        blocks.push_back(BlockIndices(from, i));
      from = i + 1;
    }
  if (from < raw_contents_.size())
    blocks.push_back(BlockIndices(from, raw_contents_.size()-1));

  for (BlockIndices &block_idx : blocks)
  {
    IdentificationRecord header;

    try
    {
      header = parse_header(block_idx.first);
    }
    catch (boost::bad_lexical_cast& e)
    {
      DBG << "Bad header " << e.what();
      DBG << "   " << raw_contents_.at(block_idx.first);
    }


    if (test(header.type & RecordType::Comments))
    {
      CommentsRecord comments;
      try
      {
        comments = CommentsRecord::from_id(header, block_idx);
      }
      catch (boost::bad_lexical_cast& e)
      {
        DBG << "Bad comment " << e.what();
        DBG << "   " << header.debug();
      }


//      DBG << comments.nuclide.verboseName() << " << " << header.debug();
    }
    else if (test(header.type & RecordType::References))
    {

    }
    else if (test(header.type & RecordType::CoulombExcitation))
    {

    }
    else if (test(header.type & RecordType::MuonicAtom))
    {

    }
    else if (test(header.type & RecordType::Reaction))
    {
      ReactionData rx;
      try {
        //      DBG << header.debug();
        rx = ReactionData::from_id(header, block_idx);
        //      DBG << header.debug() << " ---> " << rx.to_string();
        unknown_decays.insert(rx.qualifier);
      }
      catch (boost::bad_lexical_cast& e)
      {
        DBG << "Bad reaction " << e.what();
        DBG << "   " << header.debug();
      }

//      if (!rx.energy.empty())
//        DBG << "   " << header.debug() << "  -->  " << rx.to_string();

    }
    else if (test(header.type & RecordType::HiXng))
    {
//      DBG << header.debug();

    }
    else if (test(header.type & RecordType::AdoptedLevels))
      adopted_levels_[header.nuc_id] = block_idx;
    else if (test(header.type & RecordType::Decay) ||
             (test(header.type & RecordType::InelasticScattering)))
    {
      BasicDecayData decaydata;
      try
      {
        decaydata = BasicDecayData::from_id(header, block_idx);
      }
      catch (boost::bad_lexical_cast& e)
      {
        DBG << "Bad decay " << e.what();
        DBG << "   " << header.debug();
      }

      if (test(header.type & RecordType::InelasticScattering))
        DBG << "Scatteing -> " << decaydata.to_string();

      if (!decaydata.mode.valid())
      {
//        DBG
//            << "Header "
//            << "[" << std::setw(5) << block_idx.first << " - " << std::setw(5) << block_idx.second << "] "
//            << raw_contents_.at(block_idx.first)
//            << "  "
//            << header.debug()
//               ;

        continue;
      }

      boost::regex filter("^[\\s0-9A-Z]{5,5}\\s\\sP[\\s0-9].*$");
      for (size_t i=block_idx.first; i < block_idx.last; ++i)
      {
        if (boost::regex_match(raw_contents_.at(i), filter))
        {
          decaydata.parents.push_back(ParentRecord::from_ensdf(raw_contents_.at(i)));
//                    DBG << "  Parent         " << decaydata.parents.back().to_string();
        }
//                else
//                  DBG << "             DEC " << raw_contents_.at(i);
      }

      if (decaydata.parents.empty())
      {
        DBG <<   " BROKEN RECORD FOR " << decaydata.to_string();
        // broken records. skipping
        continue;
      }

      // create decay string
      //   get reference to daughter map to work with (create and insert if necessary)
      std::map<std::string, BasicDecayData> &decmap = decays_[decaydata.daughter];

      std::vector<std::string> hlstrings;
      for (const ParentRecord &prec : decaydata.parents) {
        if (prec.nuclide == decaydata.parents.at(0).nuclide) // check "same parent/different half-life" scheme
          hlstrings.push_back(prec.hl.to_string(false));
      }

      const ParentRecord &prec(decaydata.parents.at(0));
      std::string decayname  = prec.nuclide.symbolicName();
      if (prec.energy > 0.0)
        decayname += "m";
      decayname += " → " + decaydata.mode.to_string(); //HACK Types
      if (!hlstrings.empty())
        decayname += ", " + join(hlstrings, " + ");

      // insert into decay map
      while (decmap.count(decayname))
        decayname += " (alt.)";
      decmap[decayname] = decaydata;
    }
    else
    {
//      if (header.extended_dsid.at(0) == '(')
//              DBG
      //            << "Header "
      //            << "[" << std::setw(5) << block_idx.first << " - " << std::setw(5) << block_idx.second << "] "
      //            << raw_contents_.at(block_idx.first)
      //            << "  "
//                  << header.debug()
//                     ;
        DBG << "Unknown header -- " << header.debug();

//              auto rxd = ReactionData::from_id(header, block_idx);

      //      DBG << "Unprocessed block begin " << block.first;
      //      for (size_t i=block.first; i < block.second; ++i)
      //        DBG << raw_contents_.at(i);
      //      DBG << "Unprocessed block end " << block.second;
    }

  }

}

