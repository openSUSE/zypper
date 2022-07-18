/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

/** \file PackageArgs.cc
 *
 */

#include <iostream>
#include <zypp/base/Logger.h>

#include "PackageArgs.h"
#include "Zypper.h"
#include "repos.h"

PackageArgs::PackageArgs( const std::vector<std::string> & args, const ResKind & kind, const Options & opts )
: zypper( Zypper::instance() )
, _opts( opts )
{
  preprocess( args );
  argsToCaps( kind );
}

// ---------------------------------------------------------------------------

void PackageArgs::preprocess( const std::vector<std::string> & args )
{
  // Preprocess asserts not to store empty strings in _args !
  auto storeIfNotEmpty = [this]( std::string && arg ) {
    if ( ! arg.empty() )
      this->_args.insert( std::move(arg) );
  };

  std::vector<std::string>::size_type argc = args.size();
  std::string arg;
  bool op = false;
  for( unsigned i = 0; i < argc; ++i )
  {
    std::string tmp = args[i];

    if ( op )
    {
      arg += tmp;
      op = false;
      tmp.clear();
    }
    // standalone operator
    else if ( tmp == "=" || tmp == "==" || tmp == "<"
           || tmp == ">" || tmp == "<=" || tmp == ">=" )
    {
      // not at the start or the end
      if ( i && i < argc - 1 )
        op = true;
    }
    // operator at the end of a random string, e.g. 'zypper='
    else if ( tmp.find_last_of( "=<>" ) == tmp.size() - 1 && i < argc - 1 )
    {
      storeIfNotEmpty( std::move(arg) );
      arg = tmp;
      op = true;
      continue;
    }
    // operator at the start of a random string e.g. '>=3.2.1'
    else if ( i && tmp.find_first_of( "=<>" ) == 0 )
    {
      arg += tmp;
      tmp.clear();
      op = false;
    }

    if ( op )
      arg += tmp;
    else
    {
      storeIfNotEmpty( std::move(arg) );
      arg = tmp;
    }
  }

  storeIfNotEmpty( std::move(arg) );

  DBG << "args received: ";
  std::copy( args.begin(), args.end(), std::ostream_iterator<std::string>(DBG, " ") );
  DBG << endl;

  DBG << "args compiled: ";
  std::copy( _args.begin(), _args.end(), std::ostream_iterator<std::string>(DBG, " ") );
  DBG << endl;
}

// ---------------------------------------------------------------------------

static bool remove_duplicate( PackageArgs::PackageSpecSet & set, const PackageSpec & obj )
{
  PackageArgs::PackageSpecSet::iterator match = set.find( obj );
  if ( match != set.end() )
  {
    DBG << "found dupe: '" << match->orig_str << "' : " << obj.orig_str << endl;
    set.erase( match );
    return true;
  }
  return false;
}

// ---------------------------------------------------------------------------

void PackageArgs::argsToCaps( const ResKind & kind )
{
  bool dont;
  std::string arg, repo;
  for_( it, _args.begin(), _args.end() )
  {
    arg = *it;
    repo.clear();

    PackageSpec spec;
    spec.orig_str = arg;

    // For given arguments:
    //    +vim
    //    -emacs
    //    libdnet1.i586
    //    perl-devel:perl(Digest::MD5)
    //    ~non-oss:opera-2:10.1-1.2.gcc44.x86_64
    //    zypper>=1.2.15
    //
    // 1) check for and remove the install/remove modifiers
    //    vim                          (install)
    //    emacs                        (remove)
    //    perl-devel:perl(Digest::MD5) (install/remove according to command)
    //
    // 2) check for and remove the repo specifier at the beginning of the arg
    //    vim                           (no repo)
    //    libdnet1.i586                 (no repo)
    //    perl(Digest::MD5)             (perl-devel repo)
    //    opera-2:10.1-1.2.gcc44.x86_64 (non-oss repo)
    //    note: repo can be specified by number/alias/name/URI, use match_repo()
    //
    // 3) parse the rest of the string as standard zypp package specifier into
    //    a Capability using Capability::guessPackageSpec
    //                                  name, arch, op, evr, kind
    //    vim                           'vim', '', '', '', 'package'
    //    libdnet1.i586                 'libdnet', 'i586', '', '', 'package'
    //    perl(Digest::MD5)             'perl(Digest::MD5)', '', '', '', 'package'
    //    opera-2:10.1-1.2.gcc44.x86_64 'opera', 'x86_64', '=', '2:10.1-1.2.gcc44', 'package'
    //    zypper>=1.2.15                'zypper', '', '>=', '1.2.15', 'package'
    //    note: depends on whether the cap in the pool


    // check for and remove the install/remove modifiers
    // sort as do/dont

    if (arg[0] == '+' || arg[0] == '~' )
    {
      dont = false;
      arg.erase( 0, 1 );
    }
    else if ( arg[0] == '-' || arg[0] == '!' )
    {
      dont = true;
      arg.erase( 0, 1 );
    }
    else if ( _opts.do_by_default )
      dont = false;
    else
      dont = true;

    // check for and remove the 'repo:' prefix
    // ignore colons coming after '(' or '=' (bnc #433679)
    // e.g. 'perl(Digest::MD5)', or 'opera=2:10.00-4102.gcc4.shared.qt3'

    bool hasRepo = false;
    std::string::size_type pos = arg.find( ':' );
    while ( pos != std::string::npos && arg.find_first_of( "(=" ) > pos )
    {
      repo = arg.substr( 0, pos );

      if ( match_repo( zypper, repo ) )
      {
        hasRepo = true;
        arg = arg.substr( pos + 1 );
        DBG << "got repo '" << repo << "' for '" << arg << "'" << endl;
        break;
      }

      //handle the case of having one or multiple ":" in the repo alias (bsc #1041178)
      pos = arg.find( ':', pos + 1 );
    }

    // not a repo, continue as usual
    if ( !hasRepo )
      repo.clear();

    //check if we already have a package specifier in the arg
    //if we do , use that one always
    Capability parsedcap;
    ResKind kindFromPosArg = ResKind::explicitBuiltin( arg );

    if ( kindFromPosArg == ResKind::nokind && kind != ResKind::nokind && kind != ResKind::package) {
      // prepend the kind for non-packages if not already there (bnc #640399)
      parsedcap = Capability::guessPackageSpec( kind.asString() + ":" + arg, spec.modified );
    } else {
      parsedcap = Capability::guessPackageSpec( arg, spec.modified );
    }

    if ( spec.modified )
    {
      std::string msg = str::form(_("'%s' not found in package names. Trying '%s'."),
                                  arg.c_str(),
                                  parsedcap.asString().c_str() );
      zypper.out().info( msg, Out::HIGH ); // TODO this should not be called here
      DBG << "'" << arg << "' not found, trying '" << parsedcap <<  "'" << endl;
    }

    // recognize misplaced command line options given as packages (bnc#391644)
    if ( arg[0] == '-' )
    {
      zypper.out().error( str::form(_("'%s' is not a package name or capability."), arg.c_str()) );
      zypper.setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      ZYPP_THROW( ExitRequestException("cli option given after args") );
    }

    MIL << "got " << (dont?"un":"") << "wanted '" << parsedcap << "'" << "; repo '" << repo << "'" << endl;

    // Store, but avoid duplicates in do and don't sets.
    spec.parsed_cap = parsedcap;
    spec.repo_alias = repo;
    if ( dont )
    {
      if ( !remove_duplicate( _dos, spec ) )
        _donts.insert( spec );
    }
    else if ( !remove_duplicate( _donts, spec ) )
      _dos.insert( spec );
  }
}

std::ostream & operator<<( std::ostream & out, const PackageSpec & spec )
{
  out << spec.orig_str << " cap:" << spec.parsed_cap;
  if ( spec.modified )
    out << " (mod)";
  if ( !spec.repo_alias.empty() )
    out << " repo: " << spec.repo_alias;
  return out;
}
