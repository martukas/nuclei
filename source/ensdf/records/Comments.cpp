#include <ensdf/records/Comments.h>
#include <ensdf/Fields.h>
#include <util/logger.h>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <util/lexical_extensions.h>
#include <algorithm>

#include <ensdf/Translator.h>

bool CommentsRecord::match(const std::string& line, std::string rt)
{
  return match_first(line, "[cdtCDT]" + rt);
}

CommentsRecord::CommentsRecord(ENSDFData& i)
{
  const auto& line = i.read();
  if (!match(line))
    return;

  nuclide = parse_check_nid(line.substr(0, 5));
  rtype = boost::trim_copy(line.substr(7, 1));

//  auto rrtype = rtype;
//  boost::replace_all(rrtype, " ", "\\s");

//  ignore = (ctype == "Dd");

  text = extract(line);
  while (i.has_more() &&
         match_cont(i.look_ahead(), "[cdtCDT]" + rtype))
    text += extract(i.read_pop());
}

std::string CommentsRecord::extract(const std::string& line)
{
  auto ctype = line.substr(6, 1);
  bool trim = (ctype != "t") && (ctype != "T");

  std::string cdata = line.substr(9, 71);
  if (trim)
    cdata = ((line[5] == ' ') ? "" : " ")
        + boost::trim_copy(cdata);
  else
    cdata += "\n";
  if (((line[5] == '#') || (line[5] == '!')) &&
      text.size() && (text[text.size()-1] != '\n'))
  {
    cdata = "\n" + cdata;
  }
  if (line[5] == '+')
  {
    cdata = cdata.substr(1, cdata.size() - 1);
    boost::trim_right(text);
  }

  if (std::isupper(ctype[0]))
//    cdata = Translator::instance().translate1(cdata);
    cdata = adjust_case(Translator::instance().translate1(cdata));
//  if (ctype[0] == 'C')
//    cdata = adjust_case(cdata);

  return cdata;
}

std::string CommentsRecord::html() const
{
  return Translator::instance().to_html(text);
}


std::string CommentsRecord::debug() const
{
  auto ret = nuclide.symbolicName() + " COMMENT ";
  if (!rtype.empty())
    ret += "[" + rtype + "]";
  return ret + "=" + text;
}

bool CommentsRecord::valid() const
{
  return nuclide.valid();
}

std::string CommentsRecord::adjust_case(const std::string &s)
{
//  DBG << "INPUT: " << s;
  std::string ret;

  std::list<std::string> close;
  bool opened {false};
  boost::char_separator<char> sep("", "^0123456789");
  boost::tokenizer<boost::char_separator<char>> tokens(s, sep);
  for (std::string t : tokens)
  {
    if (t == "^")
      opened = true;
    else if (t.size() && is_number(t.substr(0,1)))
    {
      opened = true;
      ret += t;
    }
    else
    {
      if (!opened)
      {
        ret += boost::algorithm::to_lower_copy(t);
      }
      else if (t.size())
      {
        boost::char_separator<char> sep2("", " ");
        boost::tokenizer<boost::char_separator<char>> tokens2(t, sep2);
        int j = 0;
        for (std::string t2 : tokens2)
        {
          if (j == 0)
          {
            for (size_t i=1; i < t2.size(); ++i)
              t2[i] = std::tolower(t2[i]);
            ret += t2;
          }
          else
            ret += boost::algorithm::to_lower_copy(t2);
          j++;
        }
      }
      opened = false;
    }
  }

  return ret;
}
