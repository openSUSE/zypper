/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/HardLocksFile.cc
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

#include "zypp/target/HardLocksFile.h"
#include "zypp/PoolQueryUtil.tcc"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    void HardLocksFile::load( const Pathname & file_r, Data & data_r )
    {
      PathInfo pi( file_r );
      if ( ! pi.isFile() )
      {
        WAR << "Can't read " << pi << endl;
        return;
      }

      readPoolQueriesFromFile( file_r, std::back_inserter( data_r ) );

      MIL << "Read " << pi << endl;
    }

    void HardLocksFile::store( const Pathname & file_r, const Data & data_r )
    {
      filesystem::TmpFile tmp( filesystem::TmpFile::makeSibling( file_r ) );
      filesystem::chmod( tmp.path(), 0644 );

      writePoolQueriesToFile( tmp.path(), data_r.begin(), data_r.end() );

      if ( true ) // by now: no error info from writePoolQueriesToFile
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
    std::ostream & operator<<( std::ostream & str, const HardLocksFile & obj )
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
