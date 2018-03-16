/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/DrunkenBishop.cc
 */
#include <iostream>
//#include "zypp/base/LogTools.h"
#include "zypp/base/Flags.h"
#include "zypp/base/String.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/DrunkenBishop.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace base
  {
    ///////////////////////////////////////////////////////////////////
    namespace
    {
      /** Direction the drunken Bishop wants to move. */
      enum class Direction : std::uint8_t	// actually 2 bits
      {
	NW = 0x0,
	NE = 0x1,
	SW = 0x2,
	SE = 0x3,
      };

      /** Convert a hex digit (case insensitive) into it's (4bit) integral value.
       * \throws std::invalid_argument if char
       */
      inline std::uint8_t hexDigit( char ch_r )
      {
	switch ( ch_r )
	{
	  case 'F': case 'f': return 15;
	  case 'E': case 'e': return 14;
	  case 'D': case 'd': return 13;
	  case 'C': case 'c': return 12;
	  case 'B': case 'b': return 11;
	  case 'A': case 'a': return 10;
	  case '9': return 9;
	  case '8': return 8;
	  case '7': return 7;
	  case '6': return 6;
	  case '5': return 5;
	  case '4': return 4;
	  case '3': return 3;
	  case '2': return 2;
	  case '1': return 1;
	  case '0': return 0;
	}
	throw std::invalid_argument( str::Str() << "Not a hex digit '" << ch_r << "'" );
      }
    } // namespace
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    /// \class DrunkenBishop::Impl
    /// \brief DrunkenBishop implementation.
    ///////////////////////////////////////////////////////////////////
    class DrunkenBishop::Impl : private base::NonCopyable
    {
    public:
      /** Default is an empty board. */
      Impl()
      : _h( 0U )
      , _w( 0u )
      , _s( 0U )
      , _e( 0U )
      , _renderSSH( true )
      {}

      /** Build up a new board.
       * \throws std::invalid_argument
       */
      void compute( const std::string & data_r, const std::string & title_r, unsigned height_r = Auto, unsigned width_r = Auto )
      {
	// store rendering details
	_renderSSH = ( data_r.size() <= 32 );	// up to the ssh fingerprint size
	_fp = str::toUpper( data_r.size() <= 8 ? data_r : data_r.substr( data_r.size()-8 ) );
	_tt = title_r;

	// init the board
	_h = odd(height_r);
	_w = odd(width_r);

	if ( _h == Auto )
	{
	  if ( _renderSSH )
	  { _w = 17; _h = 9; }
	  else
	  { _w = 19; _h = 11; }
	}
	else if ( _w == Auto )
	{
	  _w = (2*_h)-1;
	}

	_board = std::vector<std::uint8_t>( _w*_h, 0 );
	_s = _w*_h/2;	// start
	_e = _s;	// current/end
	++_board[_e];

	// go
	for ( const char * ch = data_r.c_str(); *ch; /*NOOP*/ )
	{
	  std::uint8_t next4 = bite( ch );
	  // next4:     0x94
	  // bits:  10 01 01 00
	  // step:   4  3  2  1
	  static const std::uint8_t stepMask(0x3);
	  move( Direction(  next4     & stepMask ) );
	  move( Direction( (next4>>2) & stepMask ) );
	  move( Direction( (next4>>4) & stepMask ) );
	  move( Direction( (next4>>6) ) );
	}
      }

      /** Render board to a stream. */
      std::ostream & dumpOn( std::ostream & str, const std::string & prefix_r, Options options_r ) const
      {
	if ( _board.empty() )
	{
	  // "++\n"
	  // "++"
	  return str << prefix_r << "++" << endl << prefix_r << "++";
	}

	static const char * colorReset = "\033[0m";
	static const char * colorBg = "\033[48;5;242m";
	bool useColor = options_r.testFlag( USE_COLOR );

	renderTitleOn( str << prefix_r , _tt );

	for ( unsigned p = 0; p < _board.size(); ++p )
	{
	  if ( ( p % _w ) == 0 )
	  {
	    if ( p )
	      str << ( useColor ? colorReset: "" ) << '|';
	    str << endl << prefix_r << '|' << ( useColor ? colorBg : "" );
	  }
	  renderOn( str, useColor, p );
	}
	str << ( useColor ? colorReset: "" ) << '|';

	renderTitleOn( str << endl << prefix_r, _fp );
	return str;
      }

    private:
      /** Increment even width/height values. */
      static unsigned odd( unsigned val_r )
      { return( val_r == Auto ? val_r : val_r|1U ); }

      /** Get next 4 moves (8 bit) from next 2 hex digits (1st digit != '\0' asserted, 0-pad if necessary).
       * \throws std::invalid_argument if char is not a hex digit or 1st char is \c \0
       */
      static std::uint8_t bite( const char *& ch_r )
      {
	std::uint8_t ret = hexDigit( *ch_r ) << 4;
	if ( *(++ch_r) )
	  ret |= hexDigit( *(ch_r++) );
	return ret;
      }

    private:
      /** Move Bishop from \ref _e into \a direction_r and update the \ref _board. */
      void move( Direction direction_r )
      {
	switch ( direction_r )
	{
	  case Direction::NW:
	    if ( atTL() )
	      /*no move*/;
	    else if ( atT() )
	      _e -= 1;
	    else if ( atL() )
	      _e -= _w;
	    else
	      _e -= _w+1;
	    break;

	  case Direction::NE:
	    if ( atTR() )
	      /*no move*/;
	    else if ( atT() )
	      _e += 1;
	    else if ( atR() )
	      _e -= _w;
	    else
	      _e -= _w-1;
	    break;

	  case Direction::SW:
	    if ( atBL() )
	      /*no move*/;
	    else if ( atB() )
	      _e -= 1;
	    else if ( atL() )
	      _e += _w;
	    else
	      _e += _w-1;
	    break;

	  case Direction::SE:
	    if ( atBR() )
	      /*no move*/;
	    else if ( atB() )
	      _e += 1;
	    else if ( atR() )
	      _e += _w;
	    else
	      _e += _w+1;
	    break;

	  default:
	    throw std::invalid_argument( str::Str() << "Bad Direction " << unsigned(direction_r) );
	}
	// update the board
	++_board[_e];
      }

      /** Whether \ref _e is in the top left corner. */
      bool atTL() const
      { return( _e == 0 ); }

      /** Whether \ref _e is in the top right corner. */
      bool atTR() const
      { return( _e == _w-1 ); }

      /** Whether \ref _e is in the bottom left corner. */
      bool atBL() const
      { return( _e == _board.size()-_w ); }

      /** Whether \ref _e is in the bottom right corner. */
      bool atBR() const
      { return( _e == _board.size()-1 ); }

      /** Whether \ref _e is in the top row. */
      bool atT() const
      { return( _e < _w ); }

      /** Whether \ref _e is in the bottom row. */
      bool atB() const
      { return( _e >= _board.size()-_w ); }

      /** Whether \ref _e is in the left column. */
      bool atL() const
      { return( ( _e % _w ) == 0 ); }

      /** Whether \ref _e is in the right column. */
      bool atR() const
      { return( ( _e % _w ) == (_w-1) ); }

    private:
      /** ANSI color heatmap. */
      const char * color( std::uint8_t idx_r ) const
      {
	static const std::vector<const char *> colors = {
	  "",			// no coin
	  "\033[38;5;21m",	// blue (cold)
	  "\033[38;5;39m",
	  "\033[38;5;50m",
	  "\033[38;5;48m",
	  "\033[38;5;46m",	// green
	  "\033[38;5;118m",
	  "\033[38;5;190m",
	  "\033[38;5;226m",	// yellow
	  "\033[38;5;220m",
	  "\033[38;5;214m",	// orange
	  "\033[38;5;208m",
	  "\033[38;5;202m",
	  "\033[38;5;196m",	// red
	  "\033[38;5;203m",
	  "\033[38;5;210m",
	  "\033[38;5;217m",	// pink
	  "\033[38;5;224m",
	  "\033[38;5;231m",	// white (hot)
	};
#if 0
	// cycle through heat map to test all colors
	if ( ! idx_r )
	  return "";
	static unsigned i = 0;
	if ( ++i == colors.size() )
	  i = 1;
	return colors[i];
#endif
	return ( idx_r < colors.size() ? colors[idx_r] : *colors.rbegin() );
      }

      /** Render non empty title strings */
      std::ostream & renderTitleOn( std::ostream & str, const std::string & title_r ) const
      {
	std::string buffer( _w+2, '-' );
	*buffer.begin() = *buffer.rbegin() = '+';

	if ( !title_r.empty() && _w >= 2 )	// extra 2 for "[]"
	{
	  std::string::size_type tlen = std::min( title_r.size(), std::string::size_type(_w-2) );
	  std::string::size_type tpos = (_w-tlen)/2;	// not (_w-2-tlen) because buffer is size _w+2
	  buffer[tpos++] = '[';
	  for ( std::string::size_type p = 0; p < tlen; ++p, ++tpos )
	    buffer[tpos] = title_r[p];
	  buffer[tpos] = ']';
	}
	return str << buffer;
      }

      /** Render board numbers to printable chars. */
      std::ostream & renderOn( std::ostream & str, bool useColor_r, unsigned pos_r ) const
      {
	static const std::string sshSet( " .o+=*BOX@%&#/^" );
	static const std::string gpgSet( " .^:li?(fxXZ#MW&8%@" );
	const std::string & charSet( _renderSSH ? sshSet : gpgSet );

	if ( useColor_r )
	  str << color( _board[pos_r] );

	if ( pos_r == _e )
	  return str << 'E';

	if ( pos_r == _s )
	  return str << 'S';

	return str << ( _board[pos_r] < charSet.size() ? charSet[_board[pos_r]] : *charSet.rbegin() );
      }

    private:
      /** Request default width/height values. */
      static constexpr const unsigned Auto = unsigned(-1);

    private:
      std::vector<std::uint8_t> _board;	///< the board
      unsigned _h;			///< board height
      unsigned _w;			///< board with
      unsigned _s;			///< start position
      unsigned _e;			///< end position

    private:
      bool _renderSSH;			///< whether to render the ssh (or gpg) char set
      std::string _fp;			///< fingerprint to render as bottom title
      std::string _tt;			///< text to render as top title

    public:
      /** Offer default Impl. */
      static shared_ptr<Impl> nullimpl()
      {
	static shared_ptr<Impl> _nullimpl( new Impl );
	return _nullimpl;
      }
    };

    ///////////////////////////////////////////////////////////////////
    //	CLASS NAME : DrunkenBishop
    ///////////////////////////////////////////////////////////////////

    DrunkenBishop::DrunkenBishop()
    : _pimpl( Impl::nullimpl() )
    { /*nothing to compute*/ }

    DrunkenBishop::DrunkenBishop( const std::string & data_r, const std::string & title_r )
    : _pimpl( new Impl )
    { _pimpl->compute( data_r, title_r ); }

    DrunkenBishop::DrunkenBishop( const std::string & data_r, const std::string & title_r, unsigned height_r )
    : _pimpl( new Impl )
    { _pimpl->compute( data_r, title_r, height_r ); }

    DrunkenBishop::DrunkenBishop( const std::string & data_r, const std::string & title_r, unsigned height_r, unsigned width_r )
    : _pimpl( new Impl )
    { _pimpl->compute( data_r, title_r, height_r, width_r ); }

    DrunkenBishop::~DrunkenBishop()
    {}

    std::ostream & DrunkenBishop::dumpOn( std::ostream & str, const std::string & prefix_r, Options options_r ) const
    { return _pimpl->dumpOn( str, prefix_r, options_r ); }

    std::string DrunkenBishop::asString( const std::string & prefix_r, Options options_r ) const
    {
      std::ostringstream str;
      dumpOn( str, prefix_r, options_r );
      return str.str();
    }

    std::vector<std::string> DrunkenBishop::asLines( const std::string & prefix_r, Options options_r ) const
    {
      std::vector<std::string> ret;
      str::split( asString(  prefix_r, options_r ), std::back_inserter(ret), "\n" );
      return ret;
    }

  } // namespace base
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
