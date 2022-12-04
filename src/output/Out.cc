#include <unistd.h>

#include <iostream>
#include <sstream>

//#include <zypp/AutoDispose.h>

#include "Out.h"
#include "Table.h"
#include "Utf8.h"

#include "Zypper.h"

///////////////////////////////////////////////////////////////////
namespace out
{
  unsigned defaultTermwidth()
  { return Zypper::instance().out().termwidth(); }
} // namespace out
///////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//	class TermLine
////////////////////////////////////////////////////////////////////////////////

std::string TermLine::get( unsigned width_r, SplitFlags flags_r, char exp_r ) const
{
  utf8::string l(lhs);	// utf8::string::size() returns visible chars (ignores ansi SGR)!
  utf8::string r(rhs);

  if ( width_r == out::termwidthUnlimited )
    return str::Str() << l << r;	// plain string if zero width

  unsigned llen = l.size();
  unsigned rlen = r.size();
  int diff = width_r - llen - rlen;

  //AutoDispose<int> _delay( 1, ::sleep );

  if ( diff > 0 )
  {
    // expand...
    if ( ! ( flags_r.testFlag( SF_EXPAND ) && ::isatty(STDOUT_FILENO) ) )
      return str::Str() << l << r;

    if ( percentHint < 0 || percentHint > 100 )
      return str::Str() << l << std::string( diff, exp_r ) << r;

    // else:  draw % indicator
    // -------
    // <1%>===
    // .<99%>=
    // .<100%>
    if ( percentHint == 0 )
      return str::Str() << l << std::string( diff, '-' ) << r;


    unsigned pc = diff * percentHint / 100; // diff > 0 && percentHint > 0
    if ( diff < 6 )	// not enough space for fancy stuff
      return str::Str() << l <<  std::string( pc, '.' ) << std::string( diff-pc, '=' ) << r;

    // else: less boring
    std::string tag( str::Str() << '<' << percentHint << "%>" );
    pc = ( pc > tag.size() ? pc - tag.size() : 0 );
    return str::Str() << l << std::string( pc, '.' ) << tag << std::string( diff-pc-tag.size(), '=' ) << r;
  }
  else if ( diff < 0 )
  {
    // truncate...
    if ( flags_r.testFlag( SF_CRUSH ) )
    {
      if ( rlen > width_r )
        return r.substr( 0, width_r ).str();
      return str::Str() << l.substr( 0, width_r - rlen ) << r;
    }
    else if ( flags_r.testFlag( SF_SPLIT ) )
    {
      str::Str out;
      if ( llen > width_r )
        mbs_write_wrapped( out.stream(), l.str(), 0, width_r );
      else
        out << l;
      return out << "\n" << ( rlen > width_r ? r.substr( 0, width_r ) : std::string( width_r - rlen, ' ' ) + r );
    }
    // else:
    return str::Str() << l << r;
  }
  // else: fits exactly
  return str::Str() << l << r;
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
  return verbosity() < Out::NORMAL;
}

std::string Out::zyppExceptionReport(const Exception & e)
{
  return e.asUserHistory();
}

void Out::searchResult( const Table & table_r )
{
  std::cout << table_r;
}

void Out::progressEnd( const std::string & id, const std::string & label, ProgressEnd donetag_r )
{
  // translator: Shown as result tag in a progress bar: ............[done]
  static const std::string done      { _("done") };
  // translator: Shown as result tag in a progress bar: .......[attention]
  static const std::string attention { MSG_WARNINGString(_("attention")).str() };
  // translator: Shown as result tag in a progress bar: ...........[error]
  static const std::string error     { MSG_ERRORString(_("error")).str() };

  const std::string & donetag { donetag_r==ProgressEnd::done ? done : donetag_r==ProgressEnd::error ? error : attention };
  progressEnd( id, label, donetag, donetag_r==ProgressEnd::error );
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

std::string Out::Error::combine( std::string && msg_r, const Exception & ex_r )
{
  if ( msg_r.empty() )
  {
    msg_r = combine( ex_r );
  }
  else
  {
    msg_r += "\n";
    msg_r += combine( ex_r );
  }
  return std::move(msg_r);
}
std::string Out::Error::combine( const Exception & ex_r )
{ return Zypper::instance().out().zyppExceptionReport( ex_r ); }
