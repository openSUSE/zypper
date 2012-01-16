/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/misc/CheckAccessDeleted.cc
 *
*/
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"
#include "zypp/base/Exception.h"

#include "zypp/PathInfo.h"
#include "zypp/ExternalProgram.h"

#include "zypp/misc/CheckAccessDeleted.h"

using std::endl;

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::misc"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////
    //
    // lsof output lines are a sequence of NUL terminated fields,
    // where the 1st char determines the fiels type.
    //
    // (pcuL) pid command userid loginname
    // (ftkn).filedescriptor type linkcount filename
    //
    /////////////////////////////////////////////////////////////////

    /** lsof output line + files extracted so far for this PID */
    typedef std::pair<std::string,std::unordered_set<std::string>> CacheEntry;

    /** Add \c cache to \c data if the process is accessing deleted files.
     * \c pid string in \c cache is the proc line \c (pcuLR), \c iles
     * are lready in place. Always clear the \c cache.files!
    */
    inline void addDataIf( std::vector<CheckAccessDeleted::ProcInfo> & data_r, const CacheEntry & cache_r )
    {
      const auto & filelist( cache_r.second );

      if ( filelist.empty() )
        return;

      // at least one file access so keep it:
      data_r.push_back( CheckAccessDeleted::ProcInfo() );
      CheckAccessDeleted::ProcInfo & pinfo( data_r.back() );
      pinfo.files.insert( pinfo.files.begin(), filelist.begin(), filelist.end() );

      const std::string & pline( cache_r.first );
      for_( ch, pline.begin(), pline.end() )
      {
        switch ( *ch )
        {
          case 'p':
            pinfo.pid = &*(ch+1);
            break;
          case 'R':
            pinfo.ppid = &*(ch+1);
            break;
          case 'u':
            pinfo.puid = &*(ch+1);
            break;
          case 'L':
            pinfo.login = &*(ch+1);
            break;
          case 'c':
            pinfo.command = &*(ch+1);
            break;
        }
        if ( *ch == '\n' ) break;		// end of data
        do { ++ch; } while ( *ch != '\0' );	// skip to next field
      }

      if ( pinfo.command.size() == 15 )
      {
        // the command name might be truncated, so we check against /proc/<pid>/exe
        Pathname command( filesystem::readlink( Pathname("/proc")/pinfo.pid/"exe" ) );
        if ( ! command.empty() )
          pinfo.command = command.basename();
      }
      //MIL << " Take " << pinfo << endl;
    }


    /** Add file to cache if it refers to a deleted executable or library file:
     * - Either the link count \c(k) is \c 0, or no link cout is present.
     * - The type \c (t) is set to \c REG or \c DEL
     * - The filedescriptor \c (f) is set to \c txt, \c mem or \c DEL
    */
    inline void addCacheIf( CacheEntry & cache_r, const std::string & line_r, bool verbose_r  )
    {
      const char * f = 0;
      const char * t = 0;
      const char * n = 0;

      for_( ch, line_r.c_str(), ch+line_r.size() )
      {
        switch ( *ch )
        {
          case 'k':
            if ( *(ch+1) != '0' )	// skip non-zero link counts
              return;
            break;
          case 'f':
            f = ch+1;
            break;
          case 't':
            t = ch+1;
            break;
          case 'n':
            n = ch+1;
            break;
        }
        if ( *ch == '\n' ) break;		// end of data
        do { ++ch; } while ( *ch != '\0' );	// skip to next field
      }

      if ( !t || !f || !n )
        return;	// wrong filedescriptor/type/name

      if ( !(    ( *t == 'R' && *(t+1) == 'E' && *(t+2) == 'G' && *(t+3) == '\0' )
              || ( *t == 'D' && *(t+1) == 'E' && *(t+2) == 'L' && *(t+3) == '\0' ) ) )
        return;	// wrong type

      if ( !(    ( *f == 'm' && *(f+1) == 'e' && *(f+2) == 'm' && *(f+3) == '\0' )
              || ( *f == 't' && *(f+1) == 'x' && *(f+2) == 't' && *(f+3) == '\0' )
              || ( *f == 'D' && *(f+1) == 'E' && *(f+2) == 'L' && *(f+3) == '\0' )
              || ( *f == 'l' && *(f+1) == 't' && *(f+2) == 'x' && *(f+3) == '\0' ) ) )
        return;	// wrong filedescriptor type

      if ( str::contains( n, "(stat: Permission denied)" ) )
        return;	// Avoid reporting false positive due to insufficient permission.

      if ( ! verbose_r )
      {
        if ( ! ( str::contains( n, "/lib" ) || str::contains( n, "bin/" ) ) )
          return; // Try to avoid reporting false positive unless verbose.
      }

      if ( *f == 'm' || *f == 'D' )	// skip some wellknown nonlibrary memorymapped files
      {
        static const char * black[] = {
            "/SYSV"
          , "/var/run/"
          , "/dev/"
        };
        for_( it, arrayBegin( black ), arrayEnd( black ) )
        {
          if ( str::hasPrefix( n, *it ) )
            return;
        }
      }
      // Add if no duplicate
      cache_r.second.insert( n );
    }
    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  CheckAccessDeleted::size_type CheckAccessDeleted::check( bool verbose_r )
  {
    _data.clear();

    static const char* argv[] =
    {
      "lsof", "-n", "-FpcuLRftkn0", NULL
    };
    ExternalProgram prog( argv, ExternalProgram::Discard_Stderr );

    // cachemap: PID => (deleted files)
    std::map<pid_t,CacheEntry> cachemap;
    pid_t cachepid;
    for( std::string line = prog.receiveLine(); ! line.empty(); line = prog.receiveLine() )
    {
      // NOTE: line contains '\0' separeated fields!
      if ( line[0] == 'p' )
      {
	str::strtonum( line.c_str()+1, cachepid );	// line is "p<PID>\0...."
	cachemap[cachepid].first.swap( line );
      }
      else
      {
	addCacheIf( cachemap[cachepid], line, verbose_r );
      }
    }

    int ret = prog.close();
    if ( ret != 0 )
    {
      Exception err( str::form("Executing 'lsof' failed (%d).", ret) );
      err.remember( prog.execError() );
      ZYPP_THROW( err );
    }

    std::vector<ProcInfo> data;
    for ( const auto & cached : cachemap )
    {
      addDataIf( data, cached.second );
    }
    _data.swap( data );
    return _data.size();
  }

  std::string CheckAccessDeleted::findService( const Pathname & command_r )
  {
    ProcInfo p;
    p.command = command_r.basename();
    return p.service();
  }
  std::string CheckAccessDeleted::findService( const char * command_r )
  { return findService( Pathname( command_r ) ); }

  std::string CheckAccessDeleted::findService( const std::string & command_r )
  { return findService( Pathname( command_r ) ); }

  std::string CheckAccessDeleted::findService( pid_t pid_r )
  { return findService( filesystem::readlink( Pathname("/proc")/str::numstring(pid_r)/"exe" ) ); }

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  std::string CheckAccessDeleted::ProcInfo::service() const
  {
    if ( command.empty() )
      return std::string();
    // TODO: This needs to be implemented smarter... be carefull
    // as we don't know whether the target is up.

    static const Pathname initD( "/etc/init.d" );
    { // init.d script with same name
      PathInfo pi( initD/command );
      if ( pi.isFile() && pi.isX() )
        return command;
    }
    { // init.d script with name + 'd'
      std::string alt( command+"d" );
      PathInfo pi( initD/alt );
      if ( pi.isFile() && pi.isX() )
        return alt;
    }
    if ( *command.rbegin() == 'd' )
    { // init.d script with name - trailing'd'
      std::string alt( command );
      alt.erase( alt.size()-1 );
      PathInfo pi( initD/alt );
      WAR <<pi << endl;
      if ( pi.isFile() && pi.isX() )
        return alt;
    }
    return std::string();
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const CheckAccessDeleted & obj )
  {
    return dumpRange( str << "CheckAccessDeleted ",
                      obj.begin(),
                      obj.end() );
  }

   /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const CheckAccessDeleted::ProcInfo & obj )
  {
    if ( obj.pid.empty() )
      return str << "<NoProc>";

    return dumpRangeLine( str << obj.command
                              << '<' << obj.pid
                              << '|' << obj.ppid
                              << '|' << obj.puid
                              << '|' << obj.login
                              << '>',
                          obj.files.begin(),
                          obj.files.end() );
  }

 /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
