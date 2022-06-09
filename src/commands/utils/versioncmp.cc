/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "versioncmp.h"
#include "utils/flags/flagtypes.h"
#include "utils/messages.h"
#include "Zypper.h"


using namespace zypp;

VersionCompareCmd::VersionCompareCmd(std::vector<std::string> &&commandAliases_r) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    _("versioncmp (vcmp) <VERSION1> <VERSION2>"),
    // translators: command summary
    _("Compare two version strings."),
    // translators: command description
    _("Compare the versions supplied as arguments."),
    DisableAll
  )
{ }

zypp::ZyppFlags::CommandGroup VersionCompareCmd::cmdOptions() const
{
  auto that = const_cast<VersionCompareCmd *>(this);
  return {{
    { "match", 'm', ZyppFlags::NoArgument,
            ZyppFlags::BoolType( &that->_missingReleaseNrAsAnyRelease, ZyppFlags::StoreTrue, _missingReleaseNrAsAnyRelease ),
            // translators: -m, --match
            _("Takes missing release number as any release.")
    }
  }};
}

void VersionCompareCmd::doReset()
{
  _missingReleaseNrAsAnyRelease = false;
}

int VersionCompareCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  if ( positionalArgs_r.size() < 2 )
  {
    report_required_arg_missing( zypper.out(), help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }
  else if ( positionalArgs_r.size() > 2 )
  {
    report_too_many_arguments( help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  Edition lhs( positionalArgs_r[0] );
  Edition rhs( positionalArgs_r[1] );
  int result;
  if ( _missingReleaseNrAsAnyRelease )
    result = lhs.match( rhs );
  else
    result = lhs.compare( rhs );

  // be terse when talking to machines
  if ( zypper.config().terse )
  {
    zypper.out().info( str::numstring(result) );
    return ZYPPER_EXIT_OK;
  }

  // tell a human
  if (result == 0)
    zypper.out().info( str::form(_("%s matches %s"), lhs.asString().c_str(), rhs.asString().c_str() ) );
  else if ( result > 0 )
    zypper.out().info( str::form(_("%s is newer than %s"), lhs.asString().c_str(), rhs.asString().c_str() ) );
  else
    zypper.out().info( str::form(_("%s is older than %s"), lhs.asString().c_str(), rhs.asString().c_str() ) );

  if ( zypper.out().verbosity() > Out::NORMAL ) {
    auto chclass = [] ( char ch )->unsigned {
      if ( ch >= '0' && ch <= '9' )
        return 1;
      if ( (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') )
        return 2;
      if ( ch == '~' )
        return 3;
      if ( ch == '^' )
        return 4;
      return 0;
    };
    auto split = [&] ( const std::string & v, std::vector<std::string_view> & vs, std::string_view first ) {
      vs.clear();
      vs.push_back( first );
      for ( const char * s = v.c_str(); *s; ) {
        unsigned cs = chclass( *s );
        if ( cs == 0 ) {
          ++s;
          continue;
        }
        const char * e = s+1;
        while ( chclass( *e ) == cs )
          ++e;
        vs.push_back( std::string_view(s,e-s) );
        s = e;
      }
    };
    auto printpadded = [] ( std::ostream & str, const std::vector<std::string_view> & vs, const std::vector<unsigned> & width ) ->std::ostream & {
      for ( unsigned i = 0; i < vs.size(); ++i ) {
        if ( i < width.size() ) {
          for ( unsigned pad = width[i] - vs[i].size(); pad; --pad )
            str << " ";
        }
        str << vs[i] << " : ";
      }
      return str;
    };
    auto print = [&] ( std::ostream & str, const std::vector<std::string_view> & ls, const std::vector<std::string_view> & rs ) {
      std::vector<unsigned> width;
      for ( unsigned i = 0; i < ls.size() && i < rs.size(); ++i ) {
        width.push_back( std::max( ls[i].size(), rs[i].size() ) );
      }
      printpadded( str, ls, width ) << endl;
      printpadded( str, rs, width ) << endl;
    };
    std::vector<std::string_view> ls;
    std::vector<std::string_view> rs;
    std::string l { lhs.version() };
    std::string r { rhs.version() };
    auto str { zypper.out().info() };

    split( l, ls, "LHSversion:" );
    split( r, rs, "RHSversion:" );
    print( str << "", ls, rs );

    l = lhs.release();
    r = rhs.release();
    if ( l.size() || r.size() ) {
      split( l, ls, "LHSrelease:" );
      split( r, rs, "RHSrelease:" );
      print( str << "", ls, rs );
    }
  }

  return ZYPPER_EXIT_OK;
}
