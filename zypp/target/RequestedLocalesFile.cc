/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/RequestedLocalesFile.cc
 *
*/
#include <iostream>
#include <fstream>

#include "zypp/base/LogTools.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/String.h"

#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"
#include "zypp/Date.h"

#include "zypp/target/RequestedLocalesFile.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    void RequestedLocalesFile::load( const Pathname & file_r, LocaleSet & locales_r )
    {
      PathInfo pi( file_r );
      if ( ! pi.isFile() )
      {
        WAR << "Can't read " << pi << endl;
        return;
      }
      std::ifstream infile( file_r.c_str() );
      for( iostr::EachLine in( infile ); in; in.next() )
      {
        std::string l( str::trim(*in) );
        if ( ! l.empty() && l[0] != '#' )
        {
          locales_r.insert( Locale(l) );
        }
      }
      MIL << "Read " << pi << endl;
    }

    void RequestedLocalesFile::store( const Pathname & file_r, const LocaleSet & locales_r )
    {
      filesystem::TmpFile tmp( filesystem::TmpFile::makeSibling( file_r ) );
      filesystem::chmod( tmp.path(), 0644 );

      std::ofstream outs( tmp.path().c_str() );
      outs << "# zypp::RequestedLocales generated " << Date::now() << endl;
      dumpRange( outs, locales_r.begin(), locales_r.end(), "#", "\n", "\n", "\n", "#\n" );
      outs.close();

      if ( outs.good() )
      {
        filesystem::rename( tmp.path(), file_r );
        MIL << "Wrote " << PathInfo(file_r) << endl;
      }
      else
      {
        ERR << "Can't write " << PathInfo(tmp.path()) << endl;
      }
    }

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const RequestedLocalesFile & obj )
    {
      str << obj.file() << ' ';
      if ( obj._localesPtr )
        str << obj.locales();
      else
        str << "(unloaded)";
      return str;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
