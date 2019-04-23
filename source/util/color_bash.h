#pragma once

#include <string>
#include <sstream>

enum BashColor
{
  NONE = 0,
  BLACK, RED, GREEN,
  YELLOW, BLUE, MAGENTA,
  CYAN, WHITE
};

enum BashSpecial
{
  RESET = 0,
  BRIGHT, DIM, UNDER,
  BLINK, REVERSE, HIDDEN
};


inline std::string col(BashColor foreground = NONE,
                       BashColor background = NONE,
                       BashSpecial special = RESET)
{
  std::stringstream s;

#ifdef NOCOLORBASH
  return s.str();
#endif

  s << "\033[";

  if (!foreground && !background && !special)
    s << "0"; // reset colors if no params

  if (special)
  {
    s << special << "m";
    if (foreground || background)
      s << "\033[";
  }

  if (foreground)
  {
    s << 29 + foreground;
    if (background)
      s << ";";
  }

  if (background)
    s << 39 + background;

  s << "m";

  return s.str();
}
