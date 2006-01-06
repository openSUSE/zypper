/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/PackageImplIf.cc
 *
*/
#include "zypp/detail/PackageImplIf.h"
#include <iostream>

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    // Default implementation of PackageImplIf attributes,
    // as far as resonable.
    /////////////////////////////////////////////////////////////////

      Date PackageImplIf::buildtime() const
      { return Date(); }

      std::string PackageImplIf::buildhost() const
      { return std::string(); }

      Date PackageImplIf::installtime() const
      { return Date(); }

      std::string PackageImplIf::distribution() const
      { return std::string(); }

      Vendor PackageImplIf::vendor() const
      { return Vendor(); }

      Label PackageImplIf::license() const
      { return Label(); }

      std::string PackageImplIf::packager() const
      { return std::string(); }

      PackageGroup PackageImplIf::group() const
      { return PackageGroup(); }

      Changelog PackageImplIf::changelog() const
      { return Changelog(); }

      std::string PackageImplIf::url() const
      { return std::string(); }

      std::string PackageImplIf::os() const
      { return std::string(); }

      std::list<std::string> PackageImplIf::prein() const
      { return std::list<std::string>(); }

      std::list<std::string> PackageImplIf::postin() const
      { return std::list<std::string>(); }

      std::list<std::string> PackageImplIf::preun() const
      { return std::list<std::string>(); }

      std::list<std::string> PackageImplIf::postun() const
      { return std::list<std::string>(); }

      ByteCount PackageImplIf::sourcesize() const
      { return ByteCount(); }

      ByteCount PackageImplIf::archivesize() const
      { return ByteCount(); }

      std::list<std::string> PackageImplIf::authors() const
      { return std::list<std::string>(); }

      std::list<std::string> PackageImplIf::filenames() const
      { return std::list<std::string>(); }

      License PackageImplIf::licenseToConfirm() const
      { return License(); }

      // disk usage class methods

      std::ostream & operator<<( std::ostream & str, const PackageImplIf::DiskUsage::Entry & obj )
      {
        return str << obj.path << '\t' << obj._size << "; files " << obj.files;
      }

      PackageImplIf::DiskUsage::Entry PackageImplIf::DiskUsage::extract( const std::string & dirname_r )
      {
        Entry ret( dirname_r );
      
        iterator fst = begin();
        for ( ; fst != end() && !fst->isBelow( ret ); ++fst )
          ; // seek 1st equal or below
      
        if ( fst != end() ) {
          iterator lst = fst;
          for ( ; lst != end() && lst->isBelow( ret ); ++lst ) {
            // collect while below
            ret += *lst;
          }
          // remove
          _dirs.erase( fst, lst );
        }
      
        return ret;
      }

      std::ostream & operator<<( std::ostream & str, const PackageImplIf::DiskUsage & obj )
      {
        str << "Package Disk Usage {" << endl;
        for ( PackageImplIf::DiskUsage::EntrySet::const_iterator it = obj._dirs.begin(); it != obj._dirs.end(); ++it ) {
          str << "   " << *it << endl;
        }
        return str << "}";
      }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
