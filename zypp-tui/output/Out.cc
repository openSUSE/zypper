#include <unistd.h>

#include <iostream>
#include <sstream>

//#include <zypp/AutoDispose.h>

#include "Out.h"
#include "Table.h"
#include "Utf8.h"

#include "Zypper.h"

////////////////////////////////////////////////////////////////////////////////
//	class TermLine
////////////////////////////////////////////////////////////////////////////////

std::string TermLine::get( unsigned width_r, SplitFlags flags_r, char exp_r ) const
{
  utf8::string l(lhs);
  utf8::string r(rhs);

  if ( width_r == out::termwidthUnlimited )
    return zypp::str::Str() << l << r;	// plain string if zero width

  unsigned llen( l.size() - lhidden );
  unsigned rlen( r.size() - rhidden );
  int diff = width_r - llen - rlen;

  //zypp::AutoDispose<int> _delay( 1, ::sleep );

  if ( diff > 0 )
  {
    // expand...
    if ( ! ( flags_r.testFlag( SF_EXPAND ) && ::isatty(STDOUT_FILENO) ) )
      return zypp::str::Str() << l << r;

    if ( percentHint < 0 || percentHint > 100 )
      return zypp::str::Str() << l << std::string( diff, exp_r ) << r;

    // else:  draw % indicator
    // -------
    // <1%>===
    // .<99%>=
    // .<100%>
    if ( percentHint == 0 )
      return zypp::str::Str() << l << std::string( diff, '-' ) << r;


    unsigned pc = diff * percentHint / 100; // diff > 0 && percentHint > 0
    if ( diff < 6 )	// not enough space for fancy stuff
      return zypp::str::Str() << l <<  std::string( pc, '.' ) << std::string( diff-pc, '=' ) << r;

    // else: less boring
    std::string tag( zypp::str::Str() << '<' << percentHint << "%>" );
    pc = ( pc > tag.size() ? pc - tag.size() : 0 );
    return zypp::str::Str() << l << std::string( pc, '.' ) << tag << std::string( diff-pc-tag.size(), '=' ) << r;
  }
  else if ( diff < 0 )
  {
    // truncate...
    if ( flags_r.testFlag( SF_CRUSH ) )
    {
      if ( rlen > width_r )
	return r.substr( 0, width_r ).str();
      return zypp::str::Str() << l.substr( 0, width_r - rlen ) << r;
    }
    else if ( flags_r.testFlag( SF_SPLIT ) )
    {
      return zypp::str::Str() << ( llen > width_r ? l.substr( 0, width_r ) : l )
			      << "\n"
			      << ( rlen > width_r ? r.substr( 0, width_r ) : std::string( width_r - rlen, ' ' ) + r );
    }
    // else:
    return zypp::str::Str() << l << r;
  }
  // else: fits exactly
  return zypp::str::Str() << l << r;
}

////////////////////////////////////////////////////////////////////////////////
//	class Out
////////////////////////////////////////////////////////////////////////////////

constexpr Out::Type Out::TYPE_NONE;
constexpr Out::Type Out::TYPE_ALL;

Out::~Out()
{}

bool Out::progressFilter()
{
  if (verbosity() < Out::NORMAL)
      return true;
  return false;
}

std::string Out::zyppExceptionReport(const zypp::Exception & e)
{
  std::ostringstream s;
  // The Exception history is a stack! So zypp::Exception::asUserHistory() prints:
  //   This is bad!          <- Exception::asUserString()
  //   History:            -+
  //    - top level error   |<- Exception::historyAsString()
  //    - mid level error   |
  //    - first error      -+
  if (verbosity() > Out::NORMAL)
    s << e.asUserHistory();
  else
    s << e.asUserString();

  return s.str();
}

void Out::searchResult( const Table & table_r )
{
  std::cout << table_r;
}

////////////////////////////////////////////////////////////////////////////////
//	class Out::Error
////////////////////////////////////////////////////////////////////////////////

int Out::Error::report( Zypper & zypper_r ) const
{
  if ( ! ( _msg.empty() && _hint.empty() ) )
    zypper_r.out().error( _msg, _hint );
  if ( _exitcode != ZYPPER_EXIT_OK )	// ZYPPER_EXIT_OK indicates exitcode is already set.
    zypper_r.setExitCode( _exitcode );
  return zypper_r.exitCode();
}
