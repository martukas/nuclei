#include <ensdf/Translator.h>
#include <ensdf/Fields.h>
#include <util/logger.h>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <algorithm>

Translator::Translator()
{
  make_dictionary1();
  make_dictionary2();
  make_hist();
}

std::string Translator::hist_key(const std::string& s)
{
  if (hist_keys_.count(s))
    return hist_keys_.at(s);
  return s;
}

std::string Translator::hist_eval_type(const std::string& s)
{
  if (hist_eval_types_.count(s))
    return hist_eval_types_.at(s);
  return s;
}

std::string Translator::translate1(const std::string &s)
{
  std::string ret;
  // specify only the kept separators
  boost::char_separator<char> sep("", " .,;:()-=+<>/$");
  boost::tokenizer<boost::char_separator<char>> tokens(s, sep);
  for (std::string t : tokens)
  {
    for (auto r : dict1)
      if (t == r.first)
      {
        t = r.second;
        break;
      }
    ret += t;
  }
  return ret;
}

void Translator::to_camel(std::string& s)
{
  boost::algorithm::to_lower(s);
  if (s.size())
    s[0]= std::toupper(s[0]);
}


std::string Translator::auth_capitalize(const std::string& s)
{
  std::string ret;
  boost::char_separator<char> sep("", " .");
  boost::tokenizer<boost::char_separator<char>> tokens(s, sep);
  for (std::string t : tokens)
  {
    auto tt = t;
    if ((tt == "ET") || (tt == "AL") || (tt == "AND"))
      boost::algorithm::to_lower(tt);
    else
      to_camel(tt);
    ret += tt;
  }
  return ret;
}

void Translator::spaces_to_html(std::string &s)
{
  boost::replace_all(s, "\n", "<br>");
  boost::replace_all(s, " ", "&nbsp;");
}

std::string Translator::spaces_to_html_copy(const std::string &s)
{
  return boost::replace_all_copy(
        boost::replace_all_copy(s, "\n", "<br>"),
        " ", "&nbsp;");
}

std::string Translator::to_html(std::string s)
{
  for (auto t : dict2_s)
    boost::replace_all(s, t.first, t.second);

  boost::replace_all(s, "<", "&lt;");
  boost::replace_all(s, ">", "&gt;");
  spaces_to_html(s);
//  DBG << "INPUT: " << s;
  std::string ret;

  std::list<std::string> close;
//  bool preserve_case {false};
  bool opened {false};
  boost::char_separator<char> sep("", "{}");
  boost::tokenizer<boost::char_separator<char>> tokens(s, sep);
  for (std::string t : tokens)
  {
    if (t == "{")
      opened = true;
    else if (t == "}")
    {
      if (!close.empty())
      {
        ret += close.back();
        close.pop_back();
      }
    }
    else
    {
      if (!opened)
      {
        ret += t;
      }
      else if (t.size())
      {
        if (t[0] == 'I')
        {
          ret += "<i>";
          close.push_back("</i>");
          if (t.size() > 1)
            ret += t.substr(1, t.size() - 1);
        }
        else if (t[0] == 'B')
        {
          ret += "<b>";
          close.push_back("</b>");
          if (t.size() > 1)
            ret += t.substr(1, t.size() - 1);
        }
        else if (t[0] == 'U')
        {
          ret += "<u>";
          close.push_back("</u>");
          if (t.size() > 1)
            ret += t.substr(1, t.size() - 1);
        }
        else if (t[0] == '+')
        {
          ret += "<sup>";
          close.push_back("</sup>");
          if (t.size() > 1)
            ret += t.substr(1, t.size() - 1);
        }
        else if (t[0] == '-')
        {
          ret += "<sub>";
          close.push_back("</sub>");
          if (t.size() > 1)
            ret += t.substr(1, t.size() - 1);
        }
        else if (t[0] == '|')
        {
          ret += "<small>";
          close.push_back("</small>");
          if (t.size() > 1)
            ret += t.substr(1, t.size() - 1);
        }
        else if (t[0] == '~')
        {
          ret += "<big>";
          close.push_back("</big>");
          if (t.size() > 1)
            ret += t.substr(1, t.size() - 1);
        }
        opened = false;
      }
    }
  }


//  DBG << "RESULT: " << ret;

  return ret;
}

void Translator::make_hist()
{
  hist_keys_["TYP"] = "Type";
  hist_keys_["AUT"] = "Author(s)";
  hist_keys_["DAT"] = "Date of change";
  hist_keys_["CUT"] = "Literature cutoff date";
  hist_keys_["CIT"] = "Citation";
  hist_keys_["COM"] = "Comments";

  hist_eval_types_["FUL"] = "Complete revision";
  hist_eval_types_["FMT"] = "Format changes";
  hist_eval_types_["ERR"] = "Errata";
  hist_eval_types_["MOD"] = "Modified";
  hist_eval_types_["UPD"] = "Update due to scan of new literature";
  hist_eval_types_["EXP"] = "Experimental (not evaluated) data set";

}

void Translator::make_dictionary2()
{
  add_dict2('!', "&copy;", "!");
  add_dict2('\"', "&macr;", "&quot;");
  add_dict2('#', "&sect;", "&otimes;");
  add_dict2('$', "e", "$");  //mathematical e?
  add_dict2('%', "&radic;", "%");
  add_dict2('&', "&equiv;", "&amp;");
  add_dict2('\'', "&deg;", "&Aring;");
  add_dict2('(', "&larr;", "(");
  add_dict2(')', "&larr;", ")");
  add_dict2('*', "&times;", "&sdot;");
  add_dict2('+', "&plusmn;", "+");
  add_dict2(',', "&frac12;", ",");
  add_dict2('-', "&#8723;", "&minus;");
  add_dict2('.', "&prop;", ".");
  add_dict2('/', "&ne;", "/");
  add_dict2('0', "(", "0");
  add_dict2('1', ")", "1");
  add_dict2('2', "[", "2");
  add_dict2('3', "]", "3");
  add_dict2('4', "&lt;", "4"); //should be dirac
  add_dict2('5', "&gt;", "5"); //should be dirac
  add_dict2('6', "&radic;", "6");
  add_dict2('7', "&int;", "7");
  add_dict2('8', "&prod;", "8");
  add_dict2('9', "&sum;", "9");
  add_dict2(':', "&dagger;", ":");
  add_dict2(';', "&Dagger;", ";");
  add_dict2('<', "&le;", "&lt;");
  add_dict2('=', "&ne;", "=");
  add_dict2('>', "&ge;", "&gt;");
  add_dict2('?', "&asymp;", "?");
  add_dict2('@', "&infin;", "&#9679;");
  add_dict2('A', "&Alpha;", "&Auml;");
  add_dict2('B', "&Beta;", "B");
  add_dict2('C', "&Eta;", "C");
  add_dict2('D', "&Delta;", "D");
  add_dict2('E', "&Epsilon;", "&Eacute;");
  add_dict2('F', "&Phi;", "F");
  add_dict2('G', "&Gamma;", "G");
  add_dict2('H', "&Chi;", "H");
  add_dict2('I', "&Iota;", "I");
  add_dict2('J', "∼", "J");
  add_dict2('K', "&Kappa;", "K");
  add_dict2('L', "&Lambda;", "L");
  add_dict2('M', "&Mu;", "M");
  add_dict2('N', "&Nu;", "N");
  add_dict2('O', "&Omicron;", "&Ouml;");
  add_dict2('P', "&Pi;", "P");
  add_dict2('Q', "&Theta;", "&Otilde;");
  add_dict2('R', "&Rho;", "R");
  add_dict2('S', "&Sigma;", "S");
  add_dict2('T', "&Tau;", "T");
  add_dict2('U', "&upsih;", "&Uuml;");
  add_dict2('V', "&nabla;", "V");
  add_dict2('W', "&Omega;", "W");
  add_dict2('X', "&Xi;", "X");
  add_dict2('Y', "&Psi;", "Y");
  add_dict2('Z', "&Zeta;", "Z");
  add_dict2('[', "{", "[");
  add_dict2(']', "}", "]");
  add_dict2('^', "&uarr;", "^");

  add_dict2('_', "&darr;", "_");
  add_dict2('`', "&rsquo;", "&lsquo;");

  add_dict2('a', "&alpha;", "&auml;");
  add_dict2('b', "&beta;", "b");
  add_dict2('c', "&eta;", "c");
  add_dict2('d', "&delta;", "d");
  add_dict2('e', "&epsilon;", "&eacute;");
  add_dict2('f', "&phi;", "f");
  add_dict2('g', "&gamma;", "g");
  add_dict2('h', "&chi;", "&#295;");
  add_dict2('i', "&iota;", "i");
  add_dict2('j', "&isin;", "j");
  add_dict2('k', "&kappa;", "k");
  add_dict2('l', "&lambda;", "&#411;");
  add_dict2('m', "&mu;", "m");
  add_dict2('n', "&nu;", "n");
  add_dict2('o', "&omicron;", "&ouml;");
  add_dict2('p', "&pi;", "p");
  add_dict2('q', "&theta;", "&otilde;");
  add_dict2('r', "&rho;", "r");
  add_dict2('s', "&sigma;", "s");
  add_dict2('t', "&tau;", "t");
  add_dict2('u', "&upsilon;", "&uuml;");
  add_dict2('v', "?", "v");
  add_dict2('w', "&omega;", "w");
  add_dict2('x', "&xi;", "x");
  add_dict2('y', "&psi;", "y");
  add_dict2('z', "&zeta;", "z");


  add_dict2('a', "&alpha;", "&ne;");
  add_dict2('b', "&beta;", "&ne;");
  add_dict2('g', "&gamma;", "&ne;");
  add_dict2('d', "&delta;", "&ne;");
  add_dict2('f', "&phi;", "&ne;");
  add_dict2('l', "&lambda;", "&ne;");
  add_dict2('m', "&mu;", "&ne;");
  add_dict2('n', "&nu;", "&ne;");
  add_dict2('p', "&pi;", "&ne;");
  add_dict2('q', "&theta;", "&ne;");
  add_dict2('s', "&sigma;", "&ne;");
}

void Translator::add_dict2(char c, std::string alt1, std::string alt2)
{
  if ((alt1.size() > 1) || (alt1[0] != c))
  {
    std::string s {"| "};
    s[1] = c;
    dict2c1[c] = alt1;
    dict2_s[s] = alt1;
  }
  if ((alt2.size() > 1) || (alt2[0] != c))
  {
    std::string s {"~ "};
    s[1] = c;
    dict2c2[c] = alt2;
    dict2_s[s] = alt2;
  }
}

void Translator::make_dictionary1()
{
  std::map<std::string, std::string> map
  {
    {"\"A\"","\"A\""},
    {"%12C","%{+12}C"},
    {"%14C","%{+14}C"},
    {"%2B-","%2|b{+-}"},
    {"%A","%|a"},
    {"%B+A","%|b{++}|a"},
    {"%B+N","%|b{++}n"},
    {"%B+P","%|b{++}p"},
    {"%B+_","%|b{++}"},
    {"%B-2N","%|b{+-}2n"},
    {"%B-N","%|b{+-}n"},
    {"%B-P","%|b{+-}p"},
    {"%B-_","%|b{+-}"},
    {"%BEC","%|b{++}|e"},
    {"%E0","%E0"},
    {"%E2","%E2"},
    {"%EC","%|e"},
    {"%ECA","%|e|a"},
    {"%ECF","%|eF"},
    {"%ECK","%|ek"},
    {"%ECP","%|ep"},
    {"%EWSR","%EWSR"},
    {"%G","%|g"},
    {"%I","%I"},
    {"%IB","%I|b"},
    {"%IG","%I|g"},
    {"%IT","%IT"},
    {"%M1","%M1"},
    {"%N","%n"},
    {"%P","%p"},
    {"%RI","%I|g"},
    {"%SF","%SF"},
    {"(A)","(|a)"},
    {"(B)","(|b)"},
    {"(COUL.)","(Coul.)"},
    {"(CV)","(CV)"},
    {"(DOWN)","(|_)"},
    {"(H,T)","(H,T)"},
    {"(IT)","(IT)"},
    {"(T)","(t)"},
    {"(THETA,H)","(|q,H)"},
    {"(THETA,H,T,T)","(|q,H,t,T)"},
    {"(THETA,T,H)","(|q,T,H)"},
    {"(UP)","(|^)"},
    {"*","\\|*\\"},
    {"**(J+1/2)","{+(J+|,)}"},
    {"**-1","{+-1}"},
    {"**-3","{+-3}"},
    {"**-4","{+-4}"},
    {"**1/2","{+1/2}"},
    {"**1/3","{+1/3}"},
    {"**2","{+2}"},
    {"**3","{+3}"},
    {"**L","{+L}"},
    {"*A**(1/3)","|*|A{+1/3}"},
    {"*DS/DW","d|s/d|W"},
    {"*E","|*E"},
    {"*EG","E|g"},
    {"*EKC","|a(K)exp"},
    {"*G*WIDTHG0**2 ","g|G{+2}\\{-|g0}"},
    {"*G2","g{-2}"},
    {"*IB-","|*I|b{+-}"},
    {"*IE","|*I|e"},
    {"*Q","|*Q"},
    {"*R","R"},
    {"*RI","I|g"},
    {"*SIGMA","|*|s"},
    {"*SUMOF","|S"},
    {"*T1/2","|*T{-1/2}"},
    {"*TAU","|t"},
    {"*WIDTH","|G"},
    {"*WIDTHP","|G{-p}"},
    {"2B-","2|b{+-}"},
    {"2J","2J"},
    {"2N*SIGMA","2N|s"},
    {"4PI","4|p"},
    {"4PIB","4|p|b"},
    {"4PIBG","4|p|b|g"},
    {"4PIG","4|p|g"},
    {"A DECAY","|a decay"},
    {"A DECAYS","|a decays"},
    {"A SYST","|a syst"},
    {"A'","|a'"},
    {"A(THETA)","A(|q)"},
    {"A**1/3","A{+1/3}"},
    {"A**2/3","A{+2/3}"},
    {"A-DECAY","|a-decay"},
    {"A-N","A-N"},
    {"A-SYST","|a-syst"},
    {"A0","A{-0}"},
    {"A1","A{-1}"},
    {"A11","A{-11}"},
    {"A2","A{-2}"},
    {"A2/A0","A{-2}/A{-0}"},
    {"A22","A{-22}"},
    {"A2P2","A{-2}P{-2}"},
    {"A3","A{-3}"},
    {"A4","A{-4}"},
    {"A44","A{-44}"},
    {"A5","A{-5}"},
    {"A6","A{-6}"},
    {"A7","A{-7}"},
    {"A=","A="},
    {"AA","|a|a"},
    {"AA0","Aa{-0}"},
    {"AAS","AAS"},
    {"AB","AB"},
    {"ACE","(|a)(ce)"},
    {"AG","|a|g"},
    {"AJ","AJ"},
    {"ALAGA","Alaga"},
    {"ALPHA","|a"},
    {"ALPHA0","|a{-0}"},
    {"ALPHA1","|a{-1}"},
    {"ALPHA2","|a{-2}"},
    {"ALPHA3","|a{-3}"},
    {"ALPHAS","|a's"},
    {"AP","|?"},
    {"APRIL","April"},
    {"AUGER","Auger"},
    {"AUGUST","August"},
    {"AVRSQ","{ <r{+2}>}"},
    {"AXK","(|a)(K x ray)"},
    {"AY","Ay"},
    {"B","|b"},
    {"B(E0","B(E0"},
    {"B(E1","B(E1"},
    {"B(E2","B(E2"},
    {"B(E3","B(E3"},
    {"B(E4","B(E4"},
    {"B(IS","|b(IS"},
    {"B(J","B(J"},
    {"B*R","|bR"},
    {"B*RHO","B|*|r"},
    {"B+","|b{++}"},
    {"B-2N","|b{+-}2n"},
    {"B-N","|b{+-}n"},
    {"B-VIBRATIONAL","|b-vibrational"},
    {"B-_","|b{+-}"},
    {"B/A","B/A"},
    {"B0","|b{-0}"},
    {"B00","|b{-00}"},
    {"B02","|b{-02}"},
    {"B03","|b{-03}"},
    {"B04","|b{-04}"},
    {"B1","|b{-1}"},
    {"B12","|b{-12}"},
    {"B2","|b{-2}"},
    {"B2*R","|b{-2}R"},
    {"B20","|b{-20}"},
    {"B22","|b{-22}"},
    {"B24","|b{-24}"},
    {"B3","|b{-3}"},
    {"B3*R","|b{-3}R"},
    {"B30","|b{-30}"},
    {"B4","|b{-4}"},
    {"B4*R","|b{-4}R"},
    {"B42","|b{-42}"},
    {"B4C","B{-4}C"},
    {"B5","|b{-5}"},
    {"B5*R","|b{-5}R"},
    {"B6","|b{-6}"},
    {"B6*R","|b{-6}R"},
    {"B7","|b{-7}"},
    {"B=","B="},
    {"BA","|b|a"},
    {"BAVRSQ","{ <|b{+2}>{+1/2}}"},
    {"BB","|b|b"},
    {"BC","|bc"},
    {"BCE","|bce"},
    {"BCS","BCS"},
    {"BE(L)","BE(L)"},
    {"BE-","|be{+-}"},
    {"BE0","B(E0)"},
    {"BE0W","B(E0)(W.u.)"},
    {"BE1","B(E1)"},
    {"BE1UP","B(E1)|^"},
    {"BE1W","B(E1)(W.u.)"},
    {"BE2","B(E2)"},
    {"BE2DWN","B(E2)|_"},
    {"BE2UP","B(E2)|^"},
    {"BE2W","B(E2)(W.u.)"},
    {"BE3","B(E3)"},
    {"BE3UP","B(E3)|^"},
    {"BE3W","B(E3)(W.u.)"},
    {"BE3WUP","B(E3)(W.u.)|^"},
    {"BE4","B(E4)"},
    {"BE4UP","B(E4)|^"},
    {"BE4W","B(E4)(W.u.)"},
    {"BE5","B(E5)"},
    {"BE5W","B(E5)(W.u.)"},
    {"BE6","B(E6)"},
    {"BE6UP","B(E6)|^"},
    {"BE6W","B(E6)(W.u.)"},
    {"BE7","B(E7)"},
    {"BE7W","B(E7)(W.u.)"},
    {"BE8","B(E8)"},
    {"BEC DECAY","|b{++}|e Decay"},
    {"BEL","B(EL)"},
    {"BELW","B(EL)(W.u.)"},
    {"BERKELEY","Berkeley"},
    {"BESSEL","Bessel"},
    {"BETA","|b"},
    {"BETA*R","|bR"},
    {"BETAS","|b's"},
    {"BETHE","Bethe"},
    {"BF3","BF{-3}"},
    {"BG","|b|g"},
    {"BGG","|b|g|g"},
    {"BGN","|b|gn"},
    {"BGO","BGO"},
    {"BGT","|b(GT)"},
    {"BIEDENHARN","Biedenharn"},
    {"BJ**2","BJ{+2}"},
    {"BL","|b{-L}"},
    {"BL**2","|b{-L}{+2}"},
    {"BL*R","|b{-L}R"},
    {"BL*R*A**(1/3)","|b{-L}RA{+1/3}"},
    {"BLAIR","Blair"},
    {"BM(L)","BM(L)"},
    {"BM1","B(M1)"},
    {"BM1UP","B(M1)|^"},
    {"BM1W","B(M1)(W.u.)"},
    {"BM2","B(M2)"},
    {"BM2UP","B(M2)|^"},
    {"BM2W","B(M2)(W.u.)"},
    {"BM3","B(M3)"},
    {"BM3W","B(M3)(W.u.)"},
    {"BM4","B(M4)"},
    {"BM4W","B(M4)(W.u.)"},
    {"BM5W","B(M5)(W.u.)"},
    {"BM8UP","B(M8)|^"},
    {"BML","B(ML)"},
    {"BMLW","B(ML)(W.u.)"},
    {"BN","|bn"},
    {"BOHR","Bohr"},
    {"BORN","Born"},
    {"BP","|bp"},
    {"BR","Branching"},
    {"BREIT","Breit"},
    {"BRINK","Brink"},
    {"Be","Be"},
    {"C","C"},
    {"C.M.","c.m."},
    {"C12G","{+12}C|g"},
    {"C2S","C{+2}S"},
    {"CA(OH)","Ca(OH)"},
    {"CC","|a"},
    {"CCBA","CCBA"},
    {"CCC","CCC"},
    {"CE","ce"},
    {"CEB","ce|b"},
    {"CEG","ce|g"},
    {"CEK","ce(K)"},
    {"CEL","ce(L)"},
    {"CEL1","ce(L1)"},
    {"CEL12","ce(L12)"},
    {"CEL2","ce(L2)"},
    {"CEL23","ce(L23)"},
    {"CEL3","ce(L3)"},
    {"CEM","ce(M)"},
    {"CEM1","ce(M1)"},
    {"CEM2","ce(M2)"},
    {"CEM23","ce(M23)"},
    {"CEM3","ce(M3)"},
    {"CEM4","ce(M4)"},
    {"CEM45","ce(M45)"},
    {"CEM5","ce(M5)"},
    {"CEN","ce(N)"},
    {"CEN1","ce(N1)"},
    {"CEN2","ce(N2)"},
    {"CEN3","ce(N3)"},
    {"CEN4","ce(N4)"},
    {"CEN45","ce(N45)"},
    {"CEN5","ce(N5)"},
    {"CEO","ce(O)"},
    {"CEO+CEP","ce(O)+ce(P)"},
    {"CEO1","ce(O1)"},
    {"CERENKOV","Cerenkov"},
    {"CERN","CERN"},
    {"CHI","|h"},
    {"CHI**2","|h{+2}"},
    {"CK","|eK"},
    {"CL","|eL"},
    {"CLEBSCH","Clebsch"},
    {"CM","|eM"},
    {"CM2","cm{+2}"},
    {"CM3","cm{+3}"},
    {"CN","|eN"},
    {"CO","Co"},
    {"COMPTON","Compton"},
    {"CONF","configuration"},
    {"CONF=","configuration="},
    {"CORIOLIS","Coriolis"},
    {"COS2TH","cos{+2}|q"},
    {"COSTER","Coster"},
    {"COUL","Coul"},
    {"COULOMB","Coulomb"},
    {"CP","CP"},
    {"CRC","CRC"},
    {"CSI","CsI"},
    {"CURIE","Curie"},
    {"Cm","Cm"},
    {"D)","D)"},
    {"D+(Q)","D+(Q)"},
    {"D+Q","D+Q"},
    {"D3HE","d{+3}He"},
    {"DA","|DA"},
    {"DA2","|DA{-2}"},
    {"DA4","|DA{-4}"},
    {"DAVRSQ","{ |D<r{+2}>}"},
    {"DAVRSQ4","{ |D<r{+4}>}"},
    {"DAVRSQ6","{ |D<r{+6}>}"},
    {"DAVYDOV","Davydov"},
    {"DBR","branching uncertainty"},
    {"DCC","|D|a"},
    {"DCO","DCO"},
    {"DCOQ","DCOQ"},
    {"DE","|DE"},
    {"DE/DX","dE/dx"},
    {"DECEMBER","December"},
    {"DEG","\\|'"},
    {"DELTA","|D\\"},
    {"DFT","|D(log ft)"},
    {"DG","d|g"},
    {"DHF","|D(HF)"},
    {"DIA","|DI|a"},
    {"DIB","|DI|b"},
    {"DIE","|DI|e"},
    {"DISPIN","|DT"},
    {"DJ","|DJ"},
    {"DJPI","|DJ|p"},
    {"DK","|DK"},
    {"DL","|DL"},
    {"DMR","|D|d"},
    {"DN","|DN"},
    {"DNB","|D(|b-normalization)"},
    {"DNR","|D(|g-normalization)"},
    {"DNT","|D(|g+ce-normalization)"},
    {"DOMEGA","d|W"},
    {"DOPPLER","Doppler"},
    {"DPAC","DPAC"},
    {"DPAD","DPAD"},
    {"DPI","|D|p"},
    {"DQ+","|DQ(|e)"},
    {"DQ-","|DQ(|b{+-})"},
    {"DQA","|DQ(|a)"},
    {"DRI","|DI|g"},
    {"DS","|DS"},
    {"DS/DW","d|s/d|W"},
    {"DSA","DSA"},
    {"DSAM","DSAM"},
    {"DSIGMA","d|s"},
    {"DSN","|DS(n)"},
    {"DSP","|DS(p)"},
    {"DT","|DT{-1/2}"},
    {"DT1/2","|DT{-1/2}"},
    {"DTI","|DI(|g+ce)"},
    {"DUBNA","Dubna"},
    {"DWBA","DWBA"},
    {"DWIA","DWIA"},
    {"DWUCK","DWUCK"},
    {"E","E"},
    {"E'(THETA)","e'(|q)"},
    {"E(A)","E(|a)"},
    {"E(D)","E(d)"},
    {"E(E)","E(e)"},
    {"E(N)","E(n)"},
    {"E(P)","E(p)"},
    {"E(P1)","E(p{-1})"},
    {"E(P2)","E(p{-2})"},
    {"E(T)","E(t)"},
    {"E**1/2","E{+1/2}"},
    {"E**2","E{+2}"},
    {"E+","e{++}"},
    {"E+-","e{+|+}"},
    {"E-E","E-E"},
    {"E.G.","{Ie.g.}"},
    {"E/DE","E/|DE"},
    {"E0","E0"},
    {"E1","E1"},
    {"E10","E10"},
    {"E2","E2"},
    {"E3","E3"},
    {"E4","E4"},
    {"E5","E5"},
    {"E6","E6"},
    {"E7","E7"},
    {"E8","E8"},
    {"E9","E9"},
    {"EA","E|a"},
    {"EAV","av E|b"},
    {"EB","E|b"},
    {"EB-","E|b{+-}"},
    {"EBE2UP","|eB(E2)|^"},
    {"EBE3UP","|eB(E3)|^"},
    {"EB_","E|b"},
    {"EC","|e"},
    {"EC2P","|e2p"},
    {"ECA","|e|a"},
    {"ECC","|a(exp)"},
    {"ECE","E(ce)"},
    {"ECK","|eK(exp)"},
    {"ECL","|eL(exp)"},
    {"ECL1","|eL1(exp)"},
    {"ECL2","|eL2(exp)"},
    {"ECL3","|eL3(exp)"},
    {"ECM","|jM(exp)"},
    {"ECN","|jN(exp)"},
    {"ECP","|ep"},
    {"ED","E(d)"},
    {"EDE","E|DE"},
    {"EE","Ee"},
    {"EEC","E|e"},
    {"EG","E|g"},
    {"EG**3","E|g{+3}"},
    {"EG**5","E|g{+5}"},
    {"EKC","|a(K)exp"},
    {"EL","EL"},
    {"EL12C","|a(L12)exp"},
    {"EL1C","|a(L1)exp"},
    {"EL23C","|a(L23)exp"},
    {"EL2C","|a(L2)exp"},
    {"EL3C","|a(L3)exp"},
    {"ELC","|a(L)exp"},
    {"EM1C","|a(M1)exp"},
    {"EM2C","|a(M2)exp"},
    {"EM3C","|a(M3)exp"},
    {"EM4C","|a(M4)exp"},
    {"EM5C","|a(M5)exp"},
    {"EMC","|a(M)exp"},
    {"EN","E(n)"},
    {"EN1C","|a(N1)exp"},
    {"EN23C","|a(N23)exp"},
    {"EN2C","|a(N2)exp"},
    {"EN3C","|a(N3)exp"},
    {"EN4C","|a(N4)exp"},
    {"ENC","|a(N)exp"},
    {"ENDF/B-V","ENDF/B-V"},
    {"ENDF/B_","ENDF/B"},
    {"ENDOR","ENDOR"},
    {"ENGE","Enge"},
    {"EP","E(p)"},
    {"EPR","EPR"},
    {"EPSILON","|e"},
    {"EPSILONB","|eB"},
    {"ESR","ESR"},
    {"ET","E(t)"},
    {"EV","eV"},
    {"EVEN-A","even-A"},
    {"EWSR","EWSR"},
    {"EX.","ex."},
    {"E{","E{"},
    {"F+B","F+B"},
    {"F-K","F-K"},
    {"F/B","F/B"},
    {"FEBRUARY","February"},
    {"FERMI","Fermi"},
    {"FESHBACH","Feshbach"},
    {"FG","(fragment)|g"},
    {"FM","fm"},
    {"FM**-1","fm{+-1}"},
    {"FM**2","fm{+2}"},
    {"FM**4","fm{+4}"},
    {"FM-1","fm{+-1}"},
    {"FOCK","Fock"},
    {"FOURIER","Fourier"},
    {"FWHM","FWHM"},
    {"G FACTOR","g factor"},
    {"G FACTORS","g factors"},
    {"G(2+","g(2+"},
    {"G*T","gT"},
    {"G*W*WIDTHG0","gw|G{-|g0}"},
    {"G*W*WIDTHG0**2 ","gW|G{-0}\\{+2}"},
    {"G*WIDTH","g|G"},
    {"G*WIDTHG0","g|G{-|g0}"},
    {"G*WIDTHG0**2","g|G{+2}\\{-|g0}"},
    {"G*WIDTHN","g|G{-n}"},
    {"G+-","|g{+|+}"},
    {"G-FACTOR","g-factor"},
    {"G-FACTORS","g-factors"},
    {"G-M","G-M"},
    {"G/A","|g/|a"},
    {"G0","|g{-0}"},
    {"G1","g{-1}"},
    {"G1*WIDTH","g{-1}|G"},
    {"G2","g{-2}"},
    {"G2*WIDTH","g{-2}|G"},
    {"G=","g="},
    {"GA","|?>"},
    {"GA2","g{-A}\\{+2}"},
    {"GALLAGHER","Gallagher"},
    {"GAMMA","|g"},
    {"GAMOW","Gamow"},
    {"GARVEY","Garvey"},
    {"GAUSSIAN","Gaussian"},
    {"GB","|g|b"},
    {"GB-","|g|b{+-}"},
    {"GCE","|gce"},
    {"GDR","GDR"},
    {"GE","|>"},
    {"GE(LI)","Ge(Li)"},
    {"GE-","|ge{+-}"},
    {"GEIGER","Geiger"},
    {"GEIGER-MULLER ","Geiger-Muller"},
    {"GELI","Ge(Li)"},
    {"GEV","GeV"},
    {"GG","|g|g"},
    {"GGG","|g|g|g"},
    {"GGN","|g|gn"},
    {"GGT","|g|g|t"},
    {"GM","GM"},
    {"GMR","GMR"},
    {"GN","|gn"},
    {"GP","|gp"},
    {"GP'","|gp'"},
    {"GP(T)","|gp(t)"},
    {"GQR","GQR"},
    {"GS","g.s."},
    {"GSI","GSI"},
    {"GT",">"},
    {"GT1/2","gT{-1/2}"},
    {"GTOL","GTOL"},
    {"GWIDTH0WIDTHG ","g|G{-0}|G|g"},
    {"GX","|gX"},
    {"G_","|g"},
    {"H(","H("},
    {"H**2","h{+2}"},
    {"H,","H,"},
    {"H=","H="},
    {"HAGER","Hager"},
    {"HARTREE","Hartree"},
    {"HAUSER","Hauser"},
    {"HERA","HERA"},
    {"HF","HF"},
    {"HI","HI"},
    {"HOMEGA","h\\|`|w"},
    {"HP","HP"},
    {"HPGE","HPGE"},
    {"I","I"},
    {"I.E.","{Ii.e.}"},
    {"IA","I|a"},
    {"IAR","IAR"},
    {"IAS","IAS"},
    {"IB","I|b"},
    {"IB+","I|b{++}"},
    {"IB-","I|b{+-}"},
    {"IBA","IBA"},
    {"IBM","IBM"},
    {"IBS","IBS"},
    {"ICC","|a"},
    {"ICE","Ice"},
    {"ICE(K)","Ice(K)"},
    {"ICE(N)","Ice(N)"},
    {"IE","I|e"},
    {"IEC","I|e"},
    {"IG","I|g"},
    {"IG*EG","I|gE|g"},
    {"IGISOL","IGISOL"},
    {"IMPAC","IMPAC"},
    {"IN(","In("},
    {"INFNT","|@"},
    {"IPAC","IPAC"},
    {"IS D","is D"},
    {"ISOLDE","ISOLDE"},
    {"ISPIN","T"},
    {"ISPINZ","T{-z}"},
    {"IT BRANCHING","IT branching"},
    {"IT DECAY","IT decay"},
    {"IT DECAYS","IT decays"},
    {"IT TRANSITION","IT transition"},
    {"IT-","IT-"},
    {"IT=","IT="},
    {"IX","I(x ray)"},
    {"J","J"},
    {"J**2","J{+2}"},
    {"J0","J{-0}"},
    {"J1","J{-1}"},
    {"J2","J{-2}"},
    {"JANUARY","January"},
    {"JF","J{-f}"},
    {"JI","J{-i}"},
    {"JKP","JK|p"},
    {"JMAX","Jmax"},
    {"JMIN","Jmin"},
    {"JOSEF","JOSEF"},
    {"JPI","J|p"},
    {"JULIE","JULIE"},
    {"JULY","July"},
    {"JUNE","June"},
    {"K","K"},
    {"K/L+M","K/L+M"},
    {"K/LM","K/LM"},
    {"K/T","ce(K)/(|g+ce)"},
    {"KAPPA","|k"},
    {"KC","|a(K)"},
    {"KELSON","Kelson"},
    {"KEV","keV"},
    {"KEVIN","Kelvin"},
    {"KG","kG"},
    {"KL1L1","KL{-1}L{-1}"},
    {"KL1L2","KL{-1}L{-2}"},
    {"KL1L3","KL{-1}L{-3}"},
    {"KL1M1","KL{-1}M{-1}"},
    {"KL1M2","KL{-1}M{-2}"},
    {"KL1M3","KL{-1}M{-3}"},
    {"KL2L2","KL{-2}L{-2}"},
    {"KL2L3","KL{-2}L{-3}"},
    {"KL2M1","KL{-2}M{-1}"},
    {"KL2M3","KL{-2}M{-3}"},
    {"KL2M4","KL{-2}M{-4}"},
    {"KL3L3","KL{-3}L{-3}"},
    {"KL3LM1","KL{-3}LM{-1}"},
    {"KL3M2","KL{-3}M{-2}"},
    {"KL3M3","KL{-3}M{-3}"},
    {"KL3N","KL{-3}N"},
    {"KLL","KLL"},
    {"KLM","KLM"},
    {"KM2M3","KM{-2}M{-3}"},
    {"KM2N2","KM{-2}N{-2}"},
    {"KM3M3","KM{-3}M{-3}"},
    {"KNIGHT","Knight"},
    {"KOE","kOe"},
    {"KPI","K|p"},
    {"KRANE","Krane"},
    {"KRONIG","Kronig"},
    {"KUO-BROWN","Kuo-Brown"},
    {"KURIE","Kurie"},
    {"KXY","KXY"},
    {"L","L"},
    {"L+/T","ce(L+)/(|g+ce)"},
    {"L/T","ce(L)/(|g+ce)"},
    {"L1","L1"},
    {"L12","L12"},
    {"L12C","|a(L12)"},
    {"L1C","|a(L1)"},
    {"L2","L2"},
    {"L23","L23"},
    {"L23C","|a(L23)"},
    {"L2C","|a(L2)"},
    {"L3","L3"},
    {"L3C","|a(L3)"},
    {"LA","|?<"},
    {"LAMBDA","|l"},
    {"LAMPF","LAMPF"},
    {"LARMOR","Larmor"},
    {"LASER","LASER"},
    {"LBL","LBL"},
    {"LC","|a(L)"},
    {"LE","|<"},
    {"LEGENDRE","Legendre"},
    {"LI","Li"},
    {"LITHERLAND","Litherland"},
    {"LM","LM"},
    {"LMN","LMN"},
    {"LN","L(n)"},
    {"LOGF1T","log| {If{+1}t}"},
    {"LOGF1UT","log| {If{+1u}t}"},
    {"LOGF2UT","log| {If{+2u}t}"},
    {"LOGF3UT","log| {If{+3u}t}"},
    {"LOGFT","log| {Ift}"},
    {"LOHENGRIN","LOHENGRIN"},
    {"LORENTZIAN","Lorentzian"},
    {"LP","L(p)"},
    {"LT","<"},
    {"M","M"},
    {"M+/T","ce(M+)/(|g+ce)"},
    {"M+=","M+="},
    {"M-SHELL","M-shell"},
    {"M-SUBSHELL","M-subshell"},
    {"M/CE","M/total ce"},
    {"M/T","ce(M)/(|g+ce)"},
    {"M1","M1"},
    {"M12","M12"},
    {"M1C","|a(M1)"},
    {"M2","M2"},
    {"M23","M23"},
    {"M2C","|a(M2)"},
    {"M3","M3"},
    {"M3C","|a(M3)"},
    {"M4","M4"},
    {"M45","M45"},
    {"M4C","|a(M4)"},
    {"M5","M5"},
    {"M5C","|a(M5)"},
    {"M6","M6"},
    {"M8","M8"},
    {"MARCH","March"},
    {"MB","mb"},
    {"MB/SR","mb/sr"},
    {"MC","|a(M)"},
    {"MC+","|a(M+..)"},
    {"MEDLIST","MEDLIST"},
    {"MEV","MeV"},
    {"MEV**-4","MeV{E4-4}"},
    {"MG/CM2","mg/cm{+2}"},
    {"MHZ","MHZ"},
    {"MILLI-EV","meV"},
    {"MIT","MIT"},
    {"ML","M+L"},
    {"MNO","M+N+O"},
    {"MOME2","Q"},
    {"MOME3","Octupole mom(el)"},
    {"MOMM1","|m"},
    {"MOMM3","Octupole mom(mag)"},
    {"MOMM5","2{+5} mom(mag)"},
    {"MOMM7","2{+7} mom(mag)"},
    {"MOSSBAUER","Mossbauer"},
    {"MOSZKOWSKI","Moszkowski"},
    {"MR","|d"},
    {"MR**2","|d{+2}"},
    {"MS","ms"},
    {"MU","|m"},
    {"MU-","|m{+-}"},
    {"N*SIGMA","N|*|s"},
    {"N+/T","ce(N+)/(|g+ce)"},
    {"N-SHELL","N-shell"},
    {"N-SUBSHELL","N-subshell"},
    {"N-Z","N-Z"},
    {"N/T","ce(N)/(|g+ce)"},
    {"N1","N1"},
    {"N12","N12"},
    {"N123","N123"},
    {"N1C","|a(N1)"},
    {"N2","N2"},
    {"N23","N23"},
    {"N2C","|a(N2)"},
    {"N3","N3"},
    {"N3C","|a(N3)"},
    {"N4","N4"},
    {"N45","N45"},
    {"N4C","|a(N4)"},
    {"N5","N5"},
    {"N5C","|a(N5)"},
    {"N6C","|a(N6)"},
    {"N<","N<"},
    {"N=","N="},
    {"NAI","NaI"},
    {"NB","I|b normalization"},
    {"NB/SR","nb/sr"},
    {"NBS","NBS"},
    {"NC","|a(N)"},
    {"NC+","|a(N+..)"},
    {"NC2S","NC{+2}S"},
    {"NDS","Nuclear Data Sheets"},
    {"NE","|="},
    {"NE213","NE213"},
    {"NG","n|g"},
    {"NGG","n|g|g"},
    {"NILSSON","Nilsson"},
    {"NMR","NMR"},
    {"NOTE:","Note:"},
    {"NOVEMBER","November"},
    {"NP","Particle normalization"},
    {"NQR","NQR"},
    {"NR","I|g normalization"},
    {"NS*SIGMA","NS|s"},
    {"NT","I(|g+ce) normalization"},
    {"NU","|n"},
    {"NX","NX"},
    {"Ne","Ne"},
    {"O","O"},
    {"O/Q","O/Q"},
    {"O/T","ce(O)/(|g+ce)"},
    {"O1","O1"},
    {"O123","O123"},
    {"O1C","|a(O1)"},
    {"O2","O2"},
    {"O2C","|a(O2)"},
    {"O3","O3"},
    {"O3C","|a(O3)"},
    {"O4C","|a(O4)"},
    {"OCTOBER","October"},
    {"ODD-A","odd-A"},
    {"OMEGA","|w"},
    {"OMEGA**2*TAU","|w{+2}|t"},
    {"OMEGA*T","|w|t"},
    {"ORNL","ORNL"},
    {"OSIRIS","OSIRIS"},
    {"P DECAY","p decay"},
    {"P(THETA)","p(|q)"},
    {"P+/T","ce(P+)/(|g+ce)"},
    {"P-WIDTH","p-width"},
    {"P0","P{-0}"},
    {"P1","P1"},
    {"P1/2","p1/2"},
    {"P1C","|a(P1)"},
    {"P2NG","p2n|g"},
    {"PAC","PAC"},
    {"PAD","PAD"},
    {"PALPHA","p|a"},
    {"PG","p|g"},
    {"PGG","p|g|g"},
    {"PHI","|F"},
    {"PHI(P1)","|F(p{-1})"},
    {"PHI(P2)","|F(p{-2})"},
    {"PI","|p"},
    {"PI-","|p{+-}"},
    {"PIB","|p|b"},
    {"PIBG","|p|b|g"},
    {"PIG","|p|g"},
    {"PN","P{-n}"},
    {"PNG","pn|g"},
    {"PRI","|DI|g(%)"},
    {"PSI","|Y"},
    {"PWBA","PWBA"},
    {"PWIA","PWIA"},
    {"Q","Q"},
    {"Q(","Q("},
    {"Q+O","Q+O"},
    {"Q+_","Q(|e)"},
    {"Q-","Q(|b{+-})"},
    {"Q/D","Q/D"},
    {"Q22","Q{-22}"},
    {"Q2D","Q2D"},
    {"Q2DM","Q2DM"},
    {"Q3D","Q3D"},
    {"QA","Q(|a)"},
    {"QDD","QDD"},
    {"QDDM","QDDM"},
    {"QDMDQ","QDMDQ"},
    {"QMG","QMG"},
    {"QP","Q(g.s.)"},
    {"QQSP","QQSP"},
    {"QS","Q{-s}"},
    {"QSD","QSD"},
    {"R","R"},
    {"R(DCO)","R(DCO)"},
    {"R**2","r{+2}"},
    {"R**4","r{+4}"},
    {"R**6","r{+6}"},
    {"R0","r{-0}"},
    {"RDDS","RDDS"},
    {"RDM","RDM"},
    {"RHO","|r"},
    {"RHO**2","|r{+2}"},
    {"RI","I|g"},
    {"RITZ","Ritz"},
    {"ROSE","Rose"},
    {"RPA","RPA"},
    {"RUL","RUL"},
    {"RUTHERFORD","Rutherford"},
    {"RYTZ","Rytz"},
    {"S VALUE","S value"},
    {"S VALUES","S values"},
    {"S'","S'"},
    {"S(2N)","S(2n)"},
    {"S(2P)","S(2p)"},
    {"S(CE)","s(ce)"},
    {"S-1","s{+-1}"},
    {"S-FACTOR","S-factor"},
    {"S-FACTORS","S-factors"},
    {"S-VALUE","S-value"},
    {"S-VALUES","S-values"},
    {"S-WAVE","s-wave"},
    {"S/","S/"},
    {"S=","S="},
    {"SA","S(|a)"},
    {"SAXON","Saxon"},
    {"SCHMIDT","Schmidt"},
    {"SD","SD"},
    {"SDB","SDB"},
    {"SE(LI)","Se(Li)"},
    {"SELTZER","Seltzer"},
    {"SEPTEMBER","September"},
    {"SF","SF"},
    {"SI(LI)","Si(Li)"},
    {"SIGMA","|s"},
    {"SIGMA(0)","|s{-0}"},
    {"SIGMA*DE","|s|*|DE"},
    {"SIGMAG","|s{-|g}"},
    {"SIGMAN","|s{-n}"},
    {"SIGMANU","|s|n"},
    {"SIGNA","|s(n|a)"},
    {"SIGNG","|s(n|g)"},
    {"SILI","Si(Li)"},
    {"SIO","SiO"},
    {"SLIV-BAND","Sliv-Band"},
    {"SN","S(n)"},
    {"SOREQ","SOREQ"},
    {"SP","S(p)"},
    {"STEFFEN","Steffen"},
    {"STOCKHOLM","Stockholm"},
    {"SUMOF","|S\\"},
    {"SY","syst"},
    {"Sn","Sn"},
    {"T","T{-1/2}"},
    {"T)","t)"},
    {"T,","t,"},
    {"T/","T/"},
    {"T1/2","T{-1/2}"},
    {"T20","T20"},
    {"T21","T21"},
    {"T22","T22"},
    {"TAU","|t"},
    {"TDPAD","TDPAD"},
    {"TELLER","Teller"},
    {"TEMP","T"},
    {"TG","t|g"},
    {"TH","th"},
    {"THETA","|q"},
    {"THETA**2","|q{+2}"},
    {"THETA1","|q{-1}"},
    {"THETA2","|q{-2}"},
    {"THETAA","|q|a"},
    {"THETAA**2","|q|a{+2}"},
    {"THETAG","|q|g"},
    {"THETAP1**2","|q{-p1}{+2}"},
    {"THETAP2**2","|q{-p2}{+2}"},
    {"TI","I(|g+ce)"},
    {"TOF","tof"},
    {"TPAD","TPAD"},
    {"TRISTAN","TRISTAN"},
    {"TRIUMPH","TRIUMPH"},
    {"Ti","Ti"},
    {"U","U"},
    {"U2A2","U{-2}A{-2}"},
    {"UB","|mb"},
    {"UB*MEV","|mb|*MeV"},
    {"UB/SR","|mb/sr"},
    {"UG","|mg"},
    {"UG/CM","|mg/cm"},
    {"UK","UK"},
    {"UNISOR","UNISOR"},
    {"UNIV","Univ"},
    {"UNIVERSITY","University"},
    {"US","|ms"},
    {"USA","USA"},
    {"USSR","USSR"},
    {"V","V"},
    {"VAP","VAP"},
    {"W","W"},
    {"W(THETA)*G*WIDTH","w(|q)g|G{-|g0}"},
    {"W.U.","W.u."},
    {"WEISSKOPF","Weisskopf"},
    {"WIDTH","|G"},
    {"WIDTH**2","|G{+2}"},
    {"WIDTHA","|G|a"},
    {"WIDTHA0","|G{-|a0}"},
    {"WIDTHA1","|G{-|a1}"},
    {"WIDTHA2","|G{-|a2}"},
    {"WIDTHA3","|G{-|a3}"},
    {"WIDTHA4","|G{-|a4}"},
    {"WIDTHG","|G{-|g}"},
    {"WIDTHG0","|G{-|g0}"},
    {"WIDTHG0**2","|G{+2}\\{-|g0}"},
    {"WIDTHG1","|G{-|g1}"},
    {"WIDTHN","|G{-n}"},
    {"WIDTHN0","|G{-n0}"},
    {"WIDTHP","|G{-p}"},
    {"WIDTHP'","|G{-p'}"},
    {"WIDTHP0","|G{-p0}"},
    {"WIDTHP1","|G{-p1}"},
    {"WIDTHP2","|G{-p2}"},
    {"WIGNER","Wigner"},
    {"WINTHER","Winther"},
    {"X(","X("},
    {"X-RAY","x-ray"},
    {"X-RAYS","x-rays"},
    {"XG","X|g"},
    {"XK","K| x ray"},
    {"XKA","K|a| x ray"},
    {"XKA1","K|a{-1}| x ray"},
    {"XKA2","K|a{-2}| x ray"},
    {"XKB","K|b| x ray"},
    {"XKB1","K|b{-1} x ray"},
    {"XKB13","K|b{-13} x ray"},
    {"XKB1P","K|b{-1}'| x ray"},
    {"XKB2","K|b{-2} x ray"},
    {"XKB2P","K|b{-2}'| x ray"},
    {"XKB3","K|b{-3} x ray"},
    {"XKB4","K|b{-4} x ray"},
    {"XKB5","K|b{-5} x ray"},
    {"XKB5I","K|b{-5}\\{+I} x ray"},
    {"XKB5II","K|b{-5}\\{+II} x ray"},
    {"XKG","(K| x ray)|g"},
    {"XKO2","K-O{-2} x ray"},
    {"XKO23","K-O{-23} x ray"},
    {"XKO3","K-O{-3} x ray"},
    {"XL","L| x ray"},
    {"XL1","L{-1} x ray"},
    {"XL2","L{-2} x ray"},
    {"XL3","L{-3} x ray"},
    {"XLA","L{-|a} x ray"},
    {"XLA1","L|a{-1}| x ray"},
    {"XLA2","L|a{-2}| x ray"},
    {"XLB","L{-|b} x ray"},
    {"XLB1","L|b{-1}| x ray"},
    {"XLB10","L|b{-10}| x ray"},
    {"XLB15","L|b{-15}| x ray"},
    {"XLB2","L|b{-2}| x ray"},
    {"XLB215","L|b{-215}| x ray"},
    {"XLB3","L|b{-3}| x ray"},
    {"XLB4","L|b{-4}| x ray"},
    {"XLB5","L|b{-5}| x ray"},
    {"XLB6","L|b{-6}| x ray"},
    {"XLB9","L|b{-9}| x ray"},
    {"XLC","L{-|c} x ray"},
    {"XLG","L{-|g} x ray"},
    {"XLG1","L|g{-1}| x ray"},
    {"XLG2","L|g{-2}| x ray"},
    {"XLG3","L|g{-3}| x ray"},
    {"XLG4","L|g{-4}| x ray"},
    {"XLG5","L|g{-5}| x ray"},
    {"XLG6","L|g{-6}| x ray"},
    {"XLL","L{-{Sl}} x ray"},
    {"XM","M x ray"},
    {"XPYNG","xpyn|g"},
    {"XX","XX"},
    {"YTTRIUM","Y"},
    {"Z","Z"},
    {"Z>N","Z>N"},
    {"[E2]","[E2]"},
    {"[RI","[I|g"},
    {"a0","a{-0}"},
    {"|D","|D"}
  };

  std::multimap<size_t, std::pair<std::string, std::string>> mmap;
  for (auto l : map)
    mmap.insert({l.first.size(), {l.first, l.second}});

  for (auto l = mmap.rbegin(); l != mmap.rend(); ++l)
    dict1.push_back(l->second);
}
