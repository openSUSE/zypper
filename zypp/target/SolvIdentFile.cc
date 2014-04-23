/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/SolvIdentFile.cc
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

#include "zypp/target/SolvIdentFile.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    void SolvIdentFile::load( const Pathname & file_r, Data & data_r )
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
          data_r.insert( IdString(l) );
        }
      }
      MIL << "Read " << pi << endl;
    }

    void SolvIdentFile::store( const Pathname & file_r, const Data & data_r )
    {
      filesystem::TmpFile tmp( filesystem::TmpFile::makeSibling( file_r ) );
      filesystem::chmod( tmp.path(), 0644 );

      std::ofstream outs( tmp.path().c_str() );
      outs << "# " << file_r.basename() << " generated " << Date::now() << endl;
      dumpRange( outs, data_r.begin(), data_r.end(), "#", "\n", "\n", "\n", "#\n" );
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
    std::ostream & operator<<( std::ostream & str, const SolvIdentFile & obj )
    {
      str << obj.file() << ' ';
      if ( obj._dataPtr )
        str << obj.data();
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
