#include "qvalue_record.h"
#include "ensdf_types.h"
#include <sstream>
#include "custom_logger.h"
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>


bool QValueRecord::is(const std::string& line)
{
  return match_record_type(line,
                           "^[\\s0-9A-Za-z]{5}\\s{2}Q\\s.*$");
}

QValueRecord QValueRecord::parse(size_t& idx,
                                 const std::vector<std::string>& data, bool recurse)
{
  if ((idx >= data.size()) || !is(data[idx]))
    return QValueRecord();
  auto line = data[idx];

  QValueRecord ret;
  ret.nuc_id = parse_check_nid(line.substr(0, 5));

  ret.Q = parse_val_uncert(line.substr(8, 9), line.substr(19, 2));
  ret.SN = parse_val_uncert(line.substr(21, 7), line.substr(29, 2));
  ret.SP = parse_val_uncert(line.substr(31, 7), line.substr(39, 2));
  ret.QA = parse_val_uncert(line.substr(41, 7), line.substr(49, 5));

  ret.ref = boost::trim_copy(line.substr(55, 24));

  if (!recurse)
      return ret;

  bool altcomment {false};
  boost::regex filter("^[\\s0-9A-Za-z]{5}.{2}Q.*$");
  while ((idx+1 < data.size()) &&
         boost::regex_match(data[idx+1], filter))
  {
    ++idx;
    if (CommentsRecord::match(data[idx]))
    {
      auto cr = CommentsRecord(idx, data);
      if (boost::contains(cr.text, "Current evaluation has used the following Q record"))
      {
        if (altcomment)
          DBG << "<QValueRecord::parse> 2nd altcomment for " << ret.debug();
        altcomment = true;
      }
      else
        ret.comments.push_back(cr);
    }
    else
    {
      if (!altcomment)
        DBG << "<QValueRecord::parse> No altcomment for " << ret.debug();
      auto alt = QValueRecord::parse(idx, data, false);
      if (!ret.alternative)
        ret.alternative =  std::shared_ptr<QValueRecord>
            (new QValueRecord(alt));
      else
        DBG << "<QValueRecord::parse> 2nd alt for " << ret.debug();
    }
  }

  return ret;
}

std::string QValueRecord::debug() const
{
  auto ret = nuc_id.symbolicName() + " qvalue "
      + " Q=" + Q.to_string(true)
      + " SN=" + SN.to_string(true)
      + " SP=" + SP.to_string(true)
      + " QA=" + QA.to_string(true)
      + " ref=" + ref;
  if (alternative)
    ret += "\n  EvalQ=" + alternative->debug();
  for (auto c : comments)
    ret += "\n  " + c.debug();
  return ret;
}

