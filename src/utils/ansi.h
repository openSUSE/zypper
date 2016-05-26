/*-----------------------------------------------------------*- c++ -*-\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPPER_UTILS_ANSI_H
#define ZYPPER_UTILS_ANSI_H

#include <iostream>
#include <sstream>
#include <type_traits>
#include <memory>
#include <map>
#include <string>

#include <zypp/base/String.h>
using namespace zypp;

/** If output is done in colors (depends on config) */
bool do_colors();

///////////////////////////////////////////////////////////////////
namespace ansi
{
#define ZYPPER_TRACE_SGR 0
  ///////////////////////////////////////////////////////////////////
  /// \class ColorTraits<Tp_>
  /// \brief Traits class to enable custom \ref Color construction
  ///
  /// This enables using user types (usaually enums) to be used as \ref Color
  /// with \ref ColorString or \ref ColorStream classes:
  /// \code
  ///   enum ColorContext { Red, Green, Blue };
  ///
  ///   cout << "default"
  ///        << ( ColorContext::Green << "green"
  ///                                 << ColorContext::Red << "switch to red"
  ///                                 << ( ColorContext::Blue << "blue" )
  ///                                 << "still red" )
  ///        << "default" << endl;
  /// \endcode
  ///
  /// You need to define a function <tt>ansi::Color customColorCtor( EnumType enum_r )</tt>
  /// which associates enum value and \ref Color:
  /// \code
  ///  ansi::Color customColorCtor( ColorContext ctxt_r )
  ///  { return Color associated with each enum value }
  /// \endcode
  ///
  /// Then specailize \ref ColorTraits for your enum to enable using it as \ref ColorStream
  /// \code
  ///  namespace ansi
  ///  {
  ///    template<>
  ///    struct ColorTraits<ColorContext>
  ///    { enum { customColorCtor = true }; };
  ///  }
  /// \endcode
  ///
  /// This will enable your enum being impicitly converted into ansi::Color, especially
  /// when used together with \ref ColorString and \ref ColorStream. Printing your enum
  /// on a stream, will also print the associated colors SGR sequence:
  /// \code
  ///  // provided via ColorTraits:
  ///  std::ostream & operator<<( std::ostream & str, ColorContext obj )
  ///  { return str << ansi::Color( obj ); }
  /// \endcode
  ///////////////////////////////////////////////////////////////////
  template<class Tp_>
  struct ColorTraits
  { enum { customColorCtor = false }; };

  // enabled via ctor Color::Constant -> Color
  /** \relates ColorTraits<Tp_> SFINAE: hide template signatures unless enum is enabled in \ref ColorTraits */
  template <typename CCC_>
  using EnableIfCustomColorCtor = typename std::enable_if< ansi::ColorTraits<typename std::decay<CCC_>::type>::customColorCtor >::type;

  /** \relates ColorTraits<Tp_> SFINAE: hide template signatures unless enum is enabled in \ref ColorTraits */
  template <typename CCC_>
  using DisableIfCustomColorCtor = typename std::enable_if< !ansi::ColorTraits<typename std::decay<CCC_>::type>::customColorCtor >::type;

  ///////////////////////////////////////////////////////////////////
  /// \class Color
  /// \brief Various ways to define ansi SGR sequences.
  ///
  /// Any color component (\ref Attr, \ref Fg, \ref Bg) may have the
  /// value \c Uchanged, indicating that this component should remain
  /// unchanged when the terminal color is set. To mimic this use
  /// \ref operator<<= which updates a color by omitting \c Unchanged
  /// components.
  ///nocolor
  /// \note The function \ref do_colors determines if ansi SGR sequences
  /// are generated for colors at all.
  ///
  ///////////////////////////////////////////////////////////////////
  class Color
  {
  public:
    /** Color attributes */
    enum class Attr : std::uint8_t
    { Unchanged, Normal, Bright, Reverse };

    /** Foreground colors */
    enum class Fg : std::uint8_t
    { Unchanged, Default, Black, Red, Green, Yellow, Blue, Magenta, Cyan, White };

    /** Backgroud colors */
    enum class Bg : std::uint8_t
    { Unchanged, Default, Black, Red, Green, Yellow, Blue, Magenta, Cyan, White };

    /** Color unique id type */
    typedef std::uint32_t UidType;

    /** Predefined (foregreound) color contants
     * Intentionally not an <tt>enum class</tt>, so it can be used as
     * \c Color::Red, \c Color::Default, etc.
     */
    enum Constant : std::uint8_t
    {
      Black,	BrightBlack,	// BrightBlack = Darkgray
      Red,	BrightRed,
      Green,	BrightGreen,
      Yellow,	BrightYellow,	// Yellow = Brown on Standard VGA
      Blue,	BrightBlue,
      Magenta,	BrightMagenta,
      Cyan,	BrightCyan,
      White,	BrightWhite,	// White = Gray
      Default,	BrightDefault
    };

  public:
    /** Default ctor: terminal default color */
    Color()
    : _comp( Attr::Normal, Fg::Default, Bg::Default )
    {}

    Color( Attr attr_r, Fg fg_r = Fg::Unchanged, Bg bg_r = Bg::Unchanged )
    : _comp( attr_r, fg_r, bg_r )
    {}

    Color( Attr attr_r, Bg bg_r )
    : _comp( attr_r, Fg::Unchanged, bg_r )
    {}

    Color( Fg fg_r, Bg bg_r = Bg::Unchanged  )
    : _comp( Attr::Unchanged, fg_r, bg_r )
    {}

    Color( Bg bg_r )
    : _comp( Attr::Unchanged, Fg::Unchanged, bg_r )
    {}

    /** Color constant combined with background (\ref Bg::Default) */
    Color( Constant color_r, Bg bg_r = Bg::Default )
    : _comp( ( color_r % 2 ? Attr::Bright : Attr::Normal ), Fg::Default, bg_r )
    {
      switch ( color_r )
      {
	case Black:
	case BrightBlack:	_comp.fg = Fg::Black;	break;
	case Red:
	case BrightRed:		_comp.fg = Fg::Red;	break;
	case Green:
	case BrightGreen:	_comp.fg = Fg::Green;	break;
	case Yellow:
	case BrightYellow:	_comp.fg = Fg::Yellow;	break;
	case Blue:
	case BrightBlue:	_comp.fg = Fg::Blue;	break;
	case Magenta:
	case BrightMagenta:	_comp.fg = Fg::Magenta;	break;
	case Cyan:
	case BrightCyan:	_comp.fg = Fg::Cyan;	break;
	case White:
	case BrightWhite:	_comp.fg = Fg::White;	break;
	default:
	case Default:
	case BrightDefault:	break;
      }
    }

    /** Custom ctor from ColorTraits enabled type */
    template<class CCC_, typename = EnableIfCustomColorCtor<CCC_>>
    Color( CCC_ && color_r )
    : Color( customColorCtor( std::forward<CCC_>(color_r) ) )
    {}

  public:
    /** Leave everything unchanged */
    static Color nocolor()
    { return Color( UidType(0) ); }

    /** Evaluate in boolean context (not \ref nocolor) */
    explicit operator bool() const
    { return uid(); }

    /** ANSI SGR sesquence to reset all attributes
     * \note Printing this SGR sequence has the same visible effect as setting
     * \ref Color::Default. Classes supporting re-coloring, like \ref ColorString,
     * however differ between both. While Color::Default is a color to use,
     * thus to keep when re-coloring, \ref SGRReset is used as placeholder for
     * later coloring.
     */
    static const std::string & SGRReset()
    {
#if ( ZYPPER_TRACE_SGR )
      static const std::string _reset( "\033[0m[!]" );
#else
      static const std::string _reset( "\033[0m" );
#endif
      static const std::string _noreset( "" );
      if(!do_colors()) return _noreset;
      return _reset;
    }

  public:
    /** Update Color (assign components which are not \c Unchanged in rhs ) */
    Color & operator<=( Color rhs )
    {
      if ( rhs._comp.attr != Attr::Unchanged ) _comp.attr = rhs._comp.attr;
      if ( rhs._comp.fg != Fg::Unchanged ) _comp.fg = rhs._comp.fg;
      if ( rhs._comp.bg != Bg::Unchanged ) _comp.bg = rhs._comp.bg;
      return *this;
    }
    /** \overload */
    Color & operator<=( Color::Attr rhs )
    { if ( rhs != Attr::Unchanged ) _comp.attr = rhs; return *this; }
    /** \overload */
    Color & operator<=( Color::Fg rhs )
    { if ( rhs != Fg::Unchanged ) _comp.fg = rhs; return *this; }
    /** \overload */
    Color & operator<=( Color::Bg rhs )
    { if ( rhs != Bg::Unchanged ) _comp.bg = rhs; return *this; }
    /** \overload */
    inline Color & operator<=( Color::Constant rhs );

    /** Return updated color */
    Color operator<( Color rhs ) const
    { return Color(*this) <= rhs; }
    /** \overload */
    Color operator<( Color::Attr rhs ) const
    { return Color(*this) <= rhs; }
    /** \overload */
    Color operator<( Color::Fg rhs ) const
    { return Color(*this) <= rhs; }
    /** \overload */
    Color operator<( Color::Bg rhs ) const
    { return Color(*this) <= rhs; }
    /** \overload */
    inline Color operator<( Color::Constant rhs ) const;

  public:
    Attr attr() const
    { return _comp.attr; }

    Color & attr( Attr attr_r )
    { _comp.attr = attr_r; return *this; }

    Fg fg() const
    { return _comp.fg; }

    Color & fg( Fg fg_r )
    { _comp.fg = fg_r; return *this; }

    Bg bg() const
    { return _comp.bg; }

    Color & bg( Bg bg_r )
    { _comp.bg = bg_r; return *this; }

    /** Each color has a unique numeric id  */
    UidType uid() const
    { return _comp.uid; }

    /** The colors SGRsequence if \ref do_colors is \c true */
    const std::string & str() const
    { return genSGR( *this ); }

    /** The colors SGRsequence human readable */
    std::string debugstr() const
    { return genSGR( *this ).c_str()+1; }

  public:
  /** \relates Color */
  friend inline bool operator==( Color lhs, Color rhs )
  { return( lhs.uid() == rhs.uid() ); }

  /** \relates Color */
  friend inline bool operator!=( Color lhs, Color rhs )
  { return ! ( lhs == rhs ); }

  private:
    /** Return a colors SGRsequence if \ref do_colors retruns \c true */
    static std::string & genSGR( Color color_r )
    {
      static std::map<UidType,std::string> _def;

      if ( ! ( color_r && do_colors() ) )	// nocolor, all ::Unchanged, uid 0: return empty string
      {
#if ( ZYPPER_TRACE_SGR )
	std::string & ret( _def[0] );
	if ( ret.empty() )
	  ret =  "[]";
	return ret;
#else
	return _def[0];
#endif
      }

      std::string & ret( _def[color_r._comp.uid] );
      if ( ret.empty() )
      {
	ret += "\033[";
	switch ( color_r._comp.attr )
	{
	  case Attr::Normal:	ret += "22;27;";	break;
	  case Attr::Bright:	ret += "1;";		break;
	  case Attr::Reverse:	ret += "7;";		break;
	  default:
	  case Attr::Unchanged:	break;
	}
	switch ( color_r._comp.fg )
	{
	  case Fg::Black:	ret += "30;";		break;
	  case Fg::Red:		ret += "31;";		break;
	  case Fg::Green:	ret += "32;";		break;
	  case Fg::Yellow:	ret += "33;";		break;
	  case Fg::Blue:	ret += "34;";		break;
	  case Fg::Magenta:	ret += "35;";		break;
	  case Fg::Cyan:	ret += "36;";		break;
	  case Fg::White:	ret += "37;";		break;
	  case Fg::Default:	ret += "39;";		break;
	  default:
	  case Fg::Unchanged:	break;
	}
	switch ( color_r._comp.bg )
	{
	  case Bg::Black:	ret += "40;";		break;
	  case Bg::Red:		ret += "41;";		break;
	  case Bg::Green:	ret += "42;";		break;
	  case Bg::Yellow:	ret += "43;";		break;
	  case Bg::Blue:	ret += "44;";		break;
	  case Bg::Magenta:	ret += "45;";		break;
	  case Bg::Cyan:	ret += "46;";		break;
	  case Bg::White:	ret += "47;";		break;
	  case Bg::Default:	ret += "49;";		break;
	  default:
	  case Bg::Unchanged:	break;
	}
	*ret.rbegin() = 'm';	// turn trailing ';' into 'm'
#if ( ZYPPER_TRACE_SGR )
	ret += ( color_r == Color() ? "[*]" : "[@]" );
#endif
      }
      return ret;
    }

  private:
    /** ctor nocolor, all ::Unchange( lhs.uid() == rhs.uid() )d, uid 0 */
    Color ( UidType ) {}

    union Comp {
      Comp() 	// nocolor, all ::Unchanged, uid 0
      : uid( 0 )
      {}

      Comp( Attr attr_r, Fg fg_r, Bg bg_r )
      : attr( attr_r ), fg( fg_r ), bg( bg_r ), _f( 0 )
      {}

      struct {
	Color::Attr	attr;	// std::uint8_t
	Color::Fg	fg;	// std::uint8_t
	Color::Bg	bg;	// std::uint8_t
	std::uint8_t 	_f;	// std::uint8_t
      };
      UidType		uid;	// std::uint32_t
    } _comp;
  };

  template<>
  struct ColorTraits<Color::Constant>
  { enum { customColorCtor = true }; };	// enabled via ctor Color::Constant -> Color

  // Implememtation after ColorTraits<Color::Constant> instantiation !
  Color & Color::operator<=( Color::Constant rhs )	{ return *this <= Color( rhs, Bg::Unchanged ); }
  Color Color::operator<( Color::Constant rhs ) const	{ return Color(*this) <= rhs; }

  /** \relates Color Print the colors SGRsequence if \ref do_colors is \c true */
  inline std::ostream & operator<<( std::ostream & str, Color obj )
  { return str << obj.str(); }

  ///////////////////////////////////////////////////////////////////
  /// \class ColorString
  /// \brief Colored string if \ref do_colors
  ///
  /// Stores a plain std::string (which may have color codes embedded)
  /// along with a \ref Color. Retrieving the string will render
  /// all uncolored (\ref Color::nololor) parts of the string in the
  /// \ref Color.
  ///
  /// In contrary to a \ref ColorStream you can change the basic color
  /// of the string without losing embedded highlights.
  ///
  /// \note Printing a \ref ColorString renderd in \ref Color::noclolor
  /// on a \ref ColorStream will render the string in the ColorStreams
  /// color.
  ///////////////////////////////////////////////////////////////////
  class ColorString
  {
  public:
    ColorString()
    : _color( Color::nocolor() )
    {}

    /** Ctor from color */
    explicit ColorString( Color color_r )
    : _color( color_r )
    {}

    /** Ctor from string */
    explicit ColorString( const std::string & str_r )
    : _str( str_r )
    , _color( Color::nocolor() )
    {}
    /** \overload moving */
    explicit ColorString( std::string && str_r )
    : _str( std::move(str_r) )
    , _color( Color::nocolor() )
    {}

    /** Ctor from string and color */
    ColorString( const std::string & str_r, Color color_r )
    : _str( str_r )
    , _color( color_r )
    {}
    /** \overload moving */
    ColorString( std::string && str_r, Color color_r )
    : _str( std::move(str_r) )
    , _color( color_r )
    {}

    /** Ctor from color and string */
    ColorString( Color color_r, const std::string & str_r )
    : _str( str_r )
    , _color( color_r )
    {}
    /** \overload moving */
    ColorString( Color color_r, std::string && str_r )
    : _str( std::move(str_r) )
    , _color( color_r )
    {}

  public:
    /** Assign new string */
    ColorString & operator=( const std::string & str_r )
    { _str = str_r; return *this; }
    /** \overload moving */
    ColorString & operator=( std::string && str_r )
    { _str = std::move(str_r); return *this; }

    ///////////////////////////////////////////////////////////////////
    // Append via '<<' (not '+=' '+') because it's
    // strictly evaluated left-to-right:
    //
    //   ColorString sep( "-", Color::Cyan );
    //   ColorString ver( "version" );
    //
    //   ver << sep << "release";    // + prints "-" in cyan :)
    //
    //   ver += sep + "release";     // - prints "-release" in cyan :(
    //   ver += sep += "release";    // - prints "-release" in cyan :(
    //   (ver += sep) += "release";  // + but ugly syntax
    //
    /** Append a \Ref ColorString */
    ColorString & operator<<( const ColorString & rhs )
    { _str += rhs.str(); return *this; }

    /** Append a string */
    ColorString & operator<<( const std::string & str_r )
    { _str += str_r; return *this; }
    /** \overload moving */
    ColorString & operator<<( std::string && str_r )
    { _str += std::move(str_r); return *this; }

  public:
    /** Assign \ref Color */
    ColorString & operator=( Color color_r )
    { _color = color_r; return *this; }

    /** Update \ref Color */
    ColorString & operator<=( Color color_r )
    { _color <= color_r; return *this; }

    /** Return a copy with different color. */
    ColorString operator()( Color color_r ) const
    { return ColorString( _str, color_r ); }

  public:
    /** Return strings \ref Color */
    Color color() const
    { return _color; }

  public:
    /** Whether the underlying string is empty */
    bool empty() const
    { return plainstr().empty(); }

    /** Size of the underlying string */
    std::string::size_type size() const
    { return plainstr().size(); }

    /** Return the colored string if \ref do_colors */
    std::string str() const
    { return str( _color ); }
    /** \overload */
    std::string asString() const
    { return str(); }

    /** Return the string rendered in a differernt color if \ref do_colors */
    std::string str( Color color_r ) const
    {
      std::string ret( plainstr() );
      if ( do_colors() && color_r )
      {
	using str::replaceAll;
	replaceAll( ret, Color::SGRReset(), color_r.str() );
	ret = color_r.str() + ret + Color::SGRReset();
      }
#if ( ZYPPER_TRACE_SGR )
      return "[\"<]" + ret + "[>\"]";
#endif
      return ret;
    }

  public:
    /** Return the underlying plain string */
    const std::string & plainstr() const
    { return _str; }

    /** Return the underlying plain string */
    std::string & plainstr()
    { return _str; }

    /** Access the underlying plain string via \c operator* */
    const std::string & operator*() const
    { return plainstr(); }

    /** Access the underlying plain string via \c operator* */
    std::string & operator*()
    { return plainstr(); }

  private:
    std::string _str;
    Color _color;
  };

  /** \relates ColorString Print colored on ostream */
  inline std::ostream & operator<<( std::ostream & str, const ColorString & obj )
  { return str << obj.str(); }

  ///////////////////////////////////////////////////////////////////
  /// \class ColorStream
  /// \brief Colored stream output if \ref do_colors
  ///
  /// If an \c ostream& is passed to the constructor, we directly print
  /// to this steam. Otherwise an ostringstream is used as buffer until
  /// the buffered \ref ColorStream itself is printed. Printing an unbuffered
  /// steam prints an empty string.
  ///
  /// \note Printing directly to a stream, the color is active throughout the
  /// ColorStreams lifetime (set in ctor, reset in dtor).
  ///
  /// \see \ref ColorTraits<Tp_> for how to enable convenient \class ColorStream
  /// handling via an enum type.
  /// \code
  ///   ColorStream cstr( std::move( ColorContext::Red << "Error " << 42 ) );
  ///
  ///   cout << "default"
  ///        << ( ColorContext::Green << "prints green"
  ///                                 << ColorContext::Red << "switch to red"
  ///                                 << ( ColorContext::Blue << "prints blue" )
  ///                                 << "still prints red" )
  ///        << "default" << endl;
  /// \endcode
  ///
  /// \note This class is not copyable but movable.
  ///////////////////////////////////////////////////////////////////
  class ColorStream
  {
    struct nullDeleter { void operator() (void const *) const {}; };

  public:
    /** Default Ctor (\ref Color::Default) */
    ColorStream()
    {}

    /** Ctor taking a \ref Color */
    explicit ColorStream( Color color_r )
    : _color( color_r )
    {}

    /** Ctor directly printing to a \ref std::ostream (\ref Color::Default) */
    explicit ColorStream( std::ostream & direct_r  )
    : ColorStream( direct_r, Color::Default )
    {}

    /** Ctor directly printing to a \ref std::ostream in \ref Color */
    ColorStream( std::ostream & direct_r, Color color_r  )
    : _directP( &direct_r )
    , _color( color_r )
    { (*_directP) << _color; }

    /** non copyable */
    ColorStream( const ColorStream & ) = delete;
    ColorStream & operator=( const ColorStream & ) = delete;

    /** movable */
    ColorStream( ColorStream && ) = default;
    ColorStream & operator=( ColorStream && ) = default;

    ~ColorStream()
    { if ( _directP ) (*_directP) << Color::SGRReset(); }

    /** Explicit conversion to \ref std::ostream (creates buffer if not direct) */
    explicit operator std::ostream &()
    { return stream(); }

  public:
    /** Change the streams \ref Color */
    ColorStream & operator=( Color color_r )
    {
      _color = color_r;
      if ( hasStream() )
	stream() << _color;
      return *this;
    }

    /** Update the streams \ref Color */
    ColorStream & operator<=( Color color_r )
    {
      _color <= color_r;
      if ( hasStream() )
	stream() << _color;
      return *this;
    }

  public:
    /** Return streams \ref Color */
    Color color() const
    { return _color; }

  public:
    /** Return a buffered streams content as (colored) string */
    std::string str() const
    {
      std::string ret;
      if ( hasContent() )
      {
	ret = _bufferP->str();
	ret += Color::SGRReset();
      }
      return ret;
    }

  public:
    /** Printing a \ref Color (also via enum) updates the streams \ref Color */
    ColorStream & operator<<( Color color_r )
    { return operator<=( color_r ); }

    /** Printing a \ref ColorString using his \ref Color
     * \note \ref ColorString in \ref Color::nocolor is renderd in streams color
     */
    ColorStream & operator<<( const ColorString & val_r )
    { stream() << ( val_r.color() ? val_r.str() : val_r.str(_color) ) << _color; return *this; }

    /** Printing another \ref ColorStream using his \ref Color */
    ColorStream & operator<<( const ColorStream & val_r )
#if ( ZYPPER_TRACE_SGR )
    { if ( val_r.hasContent() ) stream() << "[<<]" << val_r.content() << _color << "[>>]"; return *this; }
#else
    { if ( val_r.hasContent() ) stream() << val_r.content() << _color; return *this; }
#endif

    /** All other types are printed via std::ostream */
    template<class Tp_, typename = DisableIfCustomColorCtor<Tp_>>
    ColorStream & operator<<( const Tp_ & val_r )	// ! Universal reference here would be too greedy
    { stream() << val_r; return *this; }

    /** \overload for omaip */
    ColorStream & operator<<( std::ostream & (*omanip)( std:: ostream & ) )
    { stream() << omanip; return *this; }

  public:
    /** \relates ColorStream Print colored on ostream */
    friend inline std::ostream & operator<<( std::ostream & str, const ColorStream & obj )
#if ( ZYPPER_TRACE_SGR )
    { if ( obj.hasContent() ) str << "[<<]" << obj.content() << Color::SGRReset() << "[>>]"; return str; }
#else
    { if ( obj.hasContent() ) str << obj.content() << Color::SGRReset(); return str; }
#endif

  private:
    /** Direct or non-empty buffer */
    bool hasStream() const
    { return _directP || _bufferP; }

    /** Reference to the underlying ostream (direct or auto-created buffer) */
    std::ostream & stream() const
    {
      if ( _directP )
	return *_directP;

      if ( !_bufferP )
      {
	_bufferP.reset( new std::ostringstream );
	*_bufferP << _color;
      }
      return *_bufferP;
    }

    /** Non-empty buffer (implies direct) */
    bool hasContent() const
    { return !!_bufferP; }

    /** Content of a non-empty buffered stream or empty */
    std::string content() const
    {
      std::string ret;
      if ( hasContent() )
	ret = _bufferP->str();
      return ret;
    }

  private:
    std::unique_ptr<std::ostream,nullDeleter> _directP;
    mutable std::unique_ptr<std::ostringstream> _bufferP;
    Color _color;
  };
  ///////////////////////////////////////////////////////////////////

#undef ZYPPER_TRACE_SGR
} // namespace ansi
///////////////////////////////////////////////////////////////////

// Drag them into this namespace:
using ansi::ColorString;
using ansi::ColorStream;

/** \relates ColorStream Create \ref ColorStream via \ref Color */
template<class Tp_>
inline ansi::ColorStream operator<<( ansi::Color color_r, Tp_ && val_r )
{ return std::move( ansi::ColorStream( color_r ) << std::forward<Tp_>(val_r) ); }
/** \overload for omanip */
inline ansi::ColorStream operator<<( ansi::Color color_r, std::ostream & (*omanip)( std::ostream & ) )
{ return std::move( ansi::ColorStream( color_r ) << omanip ); }

/** \relates ColorStream Create \ref ColorStream via <tt>enum << expr</tt> */
template<class CCC_, class Tp_, typename = ansi::EnableIfCustomColorCtor<CCC_> >
inline ansi::ColorStream operator<<( CCC_ && color_r, Tp_ && val_r )
{ return std::move( ansi::ColorStream( std::forward<CCC_>(color_r) ) << std::forward<Tp_>(val_r) ); }
/** \overload for omanip */
template<class CCC_, typename = ansi::EnableIfCustomColorCtor<CCC_> >
inline ansi::ColorStream operator<<( CCC_ && color_r, std::ostream & (*omanip)( std:: ostream & ) )
{ return std::move( ansi::ColorStream( std::forward<CCC_>(color_r) ) << omanip ); }

///////////////////////////////////////////////////////////////////
namespace std
{
  /** \relates ansi::Color Stream oputput for ColorTraits enabled types
   * Defined in namespace 'std' because namespace of 'CCC_' may vary
   */
  template<class CCC_, typename = ansi::EnableIfCustomColorCtor<CCC_>>
  inline ostream & operator<<( ostream & str, CCC_ && color_r )
  { return str << ansi::Color( forward<CCC_>(color_r) ); }
} // namespace std
///////////////////////////////////////////////////////////////////
#endif // ZYPPER_UTILS_ANSI_H
