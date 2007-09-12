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

    bool found = false;
    if ( fst != end() ) {
      iterator lst = fst;
      found = true;
      // return the first found, the value is sum of all subdirectories below
      ret += *lst;
      for ( ; lst != end() && lst->isBelow( ret ); ++lst ) {
      }
      // remove
      _dirs.erase( fst, lst );
    }

    // the dir entry has been found, update all parent entries
    if (found)
    {
	std::string dname = dirname_r;
	if (dname.size() > 1 && dname[0] != '/')
	{
	    dname.insert(dname.begin(), '/');
	}

	Entry tmp( dname );

	tmp._size = ret._size;
	tmp._files = ret._files;
	// subtract du from directories above
	iterator fst = begin();
	for ( ; fst != end(); ++fst )
	{
	    // add slash if it's missing
	    std::string dd = fst->path;
	    if (dd.size() > 1 && dd[0] != '/')
	    {
		dd.insert(dd.begin(), '/');
	    }

	    // update the entry
	    if (tmp.isBelow(dd))
	    {
		*fst -= tmp;
	    }
	}
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
