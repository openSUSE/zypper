/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/DiskUsage.cc
 *
*/
#include "zypp/DiskUsage.h"
#include <iostream>

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  std::ostream & operator<<( std::ostream & str, const DiskUsage::Entry & obj )
  {
    return str << obj.path << '\t' << obj._size << "; files " << obj._files;
  }

  DiskUsage::Entry DiskUsage::extract( const std::string & dirname_r )
  {
    Entry ret( dirname_r );

    iterator fst = begin();
    for ( ; fst != end() && !fst->isBelow( ret ); ++fst )
      ; // seek 1st equal or below
  
    if ( fst != end() ) {
      iterator lst = fst;
      // return the first found, the value is sum of all subdirectories below
      ret += *lst;
      for ( ; lst != end() && lst->isBelow( ret ); ++lst ) {
      }
      // remove
      _dirs.erase( fst, lst );
    }
  
    return ret;
  }

  std::ostream & operator<<( std::ostream & str, const DiskUsage & obj )
  {
    str << "Package Disk Usage {" << endl;
    for ( DiskUsage::EntrySet::const_iterator it = obj._dirs.begin(); it != obj._dirs.end(); ++it ) {
      str << "   " << *it << endl;
    }
    return str << "}";
  }


} // namespace zypp
///////////////////////////////////////////////////////////////////
