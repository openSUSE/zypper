/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef UTILS_COLORS_H_
#define UTILS_COLORS_H_

#include <iostream>
#include <string>

#define COLOR_BLACK             "\033[30m"
#define COLOR_GREY_DARK         "\033[1;30m"
#define COLOR_BLUE              "\033[34m"
#define COLOR_BLUE_LIGHT        "\033[1;34m"
#define COLOR_CYAN              "\033[36m"
#define COLOR_CYAN_LIGHT        "\033[1;36m"
#define COLOR_GREEN             "\033[32m"
#define COLOR_GREEN_LIGHT       "\033[1;32m"
#define COLOR_PURPLE            "\033[35m"
#define COLOR_PURPLE_LIGHT      "\033[1;35m"
#define COLOR_RED               "\033[31m"
#define COLOR_RED_LIGHT         "\033[1;31m"
#define COLOR_WHITE             "\033[37m"    // grey
#define COLOR_WHITE_LIGHT       "\033[1;37m"
#define COLOR_YELLOW            "\033[33m"    // brown
#define COLOR_YELLOW_LIGHT      "\033[1;33m"

#define COLOR_RESET             "\033[m"


class Color
{
public:
  explicit Color(const std::string & color_str);

  std::string parse(const std::string & value);

  std::string value() const
  { return _value; }

private:
  std::string _value;
};


typedef enum zypper_color_contexts
{
  COLOR_CONTEXT_RESULT               = 1,
  COLOR_CONTEXT_MSG_STATUS           = 2,
  COLOR_CONTEXT_MSG_ERROR            = 3,
  COLOR_CONTEXT_MSG_WARNING          = 4,
  COLOR_CONTEXT_POSITIVE             = 5,
  COLOR_CONTEXT_NEGATIVE             = 6,
  COLOR_CONTEXT_PROMPT_OPTION        = 7,
  COLOR_CONTEXT_PROMPT_SHORTHAND     = 8,
  COLOR_CONTEXT_HIGHLIGHT            = 9,
  COLOR_CONTEXT_OSDEBUG              = 10,

  COLOR_CONTEXT_DEFAULT              = -1
} ColorContext;

/** Simple check whether stdout can handle colors. */
bool has_colors();

/** If output is done in colors */
bool do_colors();

/** Get ISO terminal escape sequence for color in given \a context. */
const std::string get_color(const ColorContext context);

/**
 * Print string \a s in given color to stdout.
 *
 * \param str              stream top print on
 * \param s                string to print
 * \param color_seq        color to print with (ISO terminal escape sequence)
 * \param prev_color       color to restore after printing. If NULL,
 *                         COLOR_RESET will be used
 */
void print_color( std::ostream & str, const std::string & s, const char * color_seq, const char * prev_color = NULL );
inline void print_color( const std::string & s, const char * color_seq, const char * prev_color = NULL )
{ print_color( std::cout, s, color_seq, prev_color ); }
/** leagacy (f)print_color */
inline void fprint_color(std::ostream & str, const std::string & s, const char * ansi_color_seq, const char * prev_color = NULL)
{ print_color( str, s, ansi_color_seq, prev_color ); }

inline void print_color( std::ostream & str, const std::string & s, const ColorContext cc, const ColorContext prev_color = COLOR_CONTEXT_DEFAULT )
{ print_color( str, s, get_color(cc).c_str(), get_color(prev_color).c_str() ); }
inline void print_color( const std::string & s, const ColorContext cc, const ColorContext prev_color = COLOR_CONTEXT_DEFAULT )
{ print_color( std::cout, s, cc, prev_color ); }
/** leagacy  (f)print_color */
inline void fprint_color(std::ostream & str, const std::string & s, const ColorContext cc, const ColorContext prev_color = COLOR_CONTEXT_DEFAULT)
{ print_color( str, s, cc, prev_color ); }


template<ColorContext CC>
struct PrintColor
{
  PrintColor( std::ostream & str_r = std::cout )
  : _str( str_r )
  { if ( do_colors() ) _str << get_color( CC ); }

  ~PrintColor()
  { if ( do_colors() ) _str << get_color( COLOR_CONTEXT_DEFAULT ); }

  operator std::ostream &()
  { return _str; }

  template<class _Tp>
  std::ostream & operator<<( const _Tp & val )
  { return _str << val; }

  std::ostream & _str;
};

typedef PrintColor<COLOR_CONTEXT_POSITIVE>	colGood;	///< good news (green)
typedef PrintColor<COLOR_CONTEXT_MSG_WARNING>	colNote;	///< pay attention (magenta)
typedef PrintColor<COLOR_CONTEXT_MSG_ERROR>	colBad;		///< bad news (red)
typedef PrintColor<COLOR_CONTEXT_HIGHLIGHT>	colH;		///< highlight (cyan)

#endif /* UTILS_COLORS_H_ */
