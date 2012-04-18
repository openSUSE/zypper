#include <iostream>
#include <sstream>

#include <zypp/AutoDispose.h>

#include "Out.h"
#include "Table.h"

std::string TermLine::get( unsigned width_r, SplitFlags flags_r, char exp_r ) const
{
  std::string l(lhs);
  std::string r(rhs);

  if ( width_r == 0 )
    return l + r;	// plain sring if zero width

  unsigned llen( l.length() - lhidden );
  unsigned rlen( r.length() - rhidden );
  int diff = width_r - llen - rlen;

  if ( diff > 0 )
  {
    // expand...
    if ( ! flags_r.testFlag( SF_EXPAND ) )
      return l + r;

    if ( percentHint < 0 || percentHint > 100 )
      return l + std::string( diff, exp_r ) + r;

    // else:  draw % indicator
    // -------
    // <1%>===
    // .<99%>=
    // .<100%>
    if ( percentHint == 0 )
      return zypp::str::Str() << l << std::string( diff, '-' ) << r;

    int pc = diff * percentHint / 100;
    if ( diff < 6 )	// not enough space for fancy stuff
      return l + std::string( pc, '.' ) + std::string( diff-pc, '=' ) + r;

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
	return r.substr( 0, width_r );
      return l.substr( 0, width_r - rlen ) + r;
    }
    else if ( flags_r.testFlag( SF_SPLIT ) )
    {
      return( ( llen > width_r ? l.substr( 0, width_r ) : l )
            + "\n"
            + ( rlen > width_r ? r.substr( 0, width_r ) : std::string( width_r - rlen, ' ' ) + r ) );
    }
    // else:
    return l + r;
  }
  // else: fits exactly
  return l + r;
}

Out::~Out()
{}

bool Out::progressFilter()
{
  if (this->verbosity() < Out::NORMAL)
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
  if (this->verbosity() > Out::NORMAL)
    s << e.asUserHistory();
  else
    s << e.asUserString();

  return s.str();
}

void Out::searchResult( const Table & table_r )
{
  std::cout << table_r;
}

