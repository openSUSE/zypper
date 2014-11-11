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


enum ColorContext
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
  COLOR_CONTEXT_LOWLIGHT             = 10,
  COLOR_CONTEXT_OSDEBUG              = 11,

  COLOR_CONTEXT_DEFAULT              = -1,

  CC_GOOD	= COLOR_CONTEXT_POSITIVE,	///< good news		(green)
  CC_NOTE	= COLOR_CONTEXT_MSG_WARNING,	///< pay attention	(magenta)
  CC_BAD	= COLOR_CONTEXT_NEGATIVE,	///< bad news		(red)
  CC_HIGH	= COLOR_CONTEXT_HIGHLIGHT,	///< highlight		(cyan)
  CC_DIM	= COLOR_CONTEXT_LOWLIGHT,	///< dim		(grey)
  CC_DEFAULT	= COLOR_CONTEXT_DEFAULT
};


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


///////////////////////////////////////////////////////////////////
/// \class ColorString
/// \brief Attributed string
/// The plain string value is attributed if colors are used and
/// if it is printed on a stream.
///////////////////////////////////////////////////////////////////
class ColorString
{
public:
  ColorString( ColorContext cc_r = CC_DEFAULT )
  : _cc( cc_r )
  {}

  ColorString( const std::string & val_r, ColorContext cc_r = CC_DEFAULT  )
  : _str( val_r )
  , _cc( cc_r )
  {}

  ColorString( const char * val_r, ColorContext cc_r = CC_DEFAULT  )
  : ColorString( std::string( val_r ? val_r : "" ), cc_r )
  {}

  template<class _Tp>
  ColorString( const _Tp & val_r, ColorContext cc_r = CC_DEFAULT  )
  : ColorString( zypp::str::asString( val_r ), cc_r )
  {}

public:
  /** Get the plain string */
  const std::string & str() const
  { return _str; }

  /** Set the plain string */
  ColorString & operator=( const std::string & rhs )
  { _str = rhs; return *this; }

  /** Set the plain string */
  ColorString & operator=( const char * rhs )
  { _str = ( rhs ? rhs : "" ); return *this; }


  /** Get the ColorContext */
  ColorContext cc() const
  { return _cc; }

  /** Set the ColorContext */
  ColorString & operator=( ColorContext rhs )
  { _cc = rhs; return *this; }


  /** Get the attribured string */
  std::string paintStr() const
  { return zypp::str::Str() << *this; }

private:
  std::string _str;
  ColorContext _cc;
};


///////////////////////////////////////////////////////////////////
/// \class Paint
/// \brief Paint atributed to stream if colors are used.
///////////////////////////////////////////////////////////////////
class Paint
{
public:
  Paint( ColorContext cc_r, std::ostream & str_r = std::cout, ColorContext reset_r = CC_DEFAULT )
  : _str( str_r )
  , _cc( cc_r )
  , _reset( reset_r )
  { if ( do_colors() ) _str << get_color( cc_r ); }

  Paint( ColorContext cc_r, ColorContext reset_r )
  : Paint( cc_r, std::cout, reset_r )
  {}

  ~Paint()
  { if ( do_colors() ) _str << get_color( _reset ); }

  operator std::ostream &()
  { return _str; }

  template<class _Tp>
  Paint & operator<<( const _Tp & val_r )
  { _str << val_r; return *this; }

  Paint & operator<<( std::ostream & (*omanip)( std:: ostream & ) )
  { _str << omanip; return *this; }

  Paint & operator<<( const ColorString & val_r )
  {
    if ( val_r.cc() != CC_DEFAULT && val_r.cc() != _cc )
      Paint( val_r.cc(), _str, _cc ) << val_r.str();
    else
      _str << val_r.str();
    return *this;
  }

private:
  std::ostream & _str;
  ColorContext _cc;
  ColorContext _reset;
};


/** \relates ColorString print attributed string */
inline std::ostream & operator<<( std::ostream & str, const ColorString & obj )
{ return Paint( obj.cc(), str ) << obj.str(); }

#endif /* UTILS_COLORS_H_ */
