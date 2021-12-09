/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/VendorAttr.cc
*/
#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include <vector>

#include <zypp/base/LogTools.h>
#include <zypp/base/IOStream.h>
#include <zypp/base/StringV.h>

#include <zypp/PathInfo.h>
#include <zypp/VendorAttr.h>
#include <zypp/ZYppFactory.h>

#include <zypp/ZConfig.h>
#include <zypp/PathInfo.h>
#include <zypp-core/parser/IniDict>

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::VendorAttr"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  /// \class VendorAttr::Impl
  /// \brief VendorAttr implementation.
  ///////////////////////////////////////////////////////////////////
  class VendorAttr::Impl // : private base::NonCopyable
  {
    friend std::ostream & operator<<( std::ostream & str, const Impl & obj );
  public:
    /** Ctor.
     * bsc#1030686: The legacy default equivalence of 'suse' and 'opensuse'
     * has been removed.
     * bnc#812608: No prefix compare in opensuse namespace, so just create
     * a class for 'suse*'
     */
    Impl()
    { _vendorGroupMap["suse"] = ++_vendorGroupId; }

  public:
    /** Add a new equivalent vendor set. */
    void addVendorList( VendorList && vendorList_r );

    /** Return whether two vendor strings should be treated as equivalent.*/
    bool equivalent( IdString lVendor, IdString rVendor ) const
    { return lVendor == rVendor || vendorMatchId( lVendor ) == vendorMatchId( rVendor ); }

    /** Return whether two vendor strings should be treated as equivalent or are (suse/opensuse).*/
    bool relaxedEquivalent( IdString lVendor, IdString rVendor ) const
    {
      if ( equivalent( lVendor, rVendor ) )
        return true;

      static const IdString suse { "suse" };
      static const IdString opensuse { "opensuse" };
      unsigned sid = vendorMatchId( suse );
      unsigned oid = vendorMatchId( opensuse );
      if ( sid == oid )
        return false; // (suse/opensuse) are equivalent, so these are not

      auto isSuse = [sid,oid]( unsigned v )-> bool { return v==sid || v==oid; };
      return isSuse( vendorMatchId(lVendor) ) && isSuse( vendorMatchId(rVendor) );
    }

    unsigned foreachVendorList( std::function<bool(VendorList)> fnc_r ) const
    {
      std::map<unsigned,VendorList> lists;
      for( const auto & el : _vendorGroupMap )
        lists[el.second].push_back( el.first );

      unsigned ret = 0;
      for ( auto el : lists ) {
        VendorList & vlist { el.second };
        if ( vlist.empty() )
          continue;
        ++ret;
        if ( fnc_r && !fnc_r( std::move(vlist) ) )
          break;
      }
      return ret;
    }

  private:
    using VendorGroupMap = std::map<std::string,unsigned>;
    VendorGroupMap _vendorGroupMap;	///< Vendor group definition. Equivalent groups share the same ID.
    unsigned _vendorGroupId = 0;	///< Highest group ID in use (incremented).

  private:
    typedef DefaultIntegral<unsigned,0>				VendorMatchEntry;
    typedef std::unordered_map<IdString, VendorMatchEntry>	VendorMatch;
    mutable VendorMatch _vendorMatch;	///< Cache mapping vendor strings to equivalence class ID
    mutable unsigned _nextId = 0;	///< Least equivalence class ID in use (decremented).

    /** Reset vendor match cache if _vendorGroupMap was changed. */
    void vendorMatchIdReset()
    {
      _nextId = 0;
      _vendorMatch.clear();
    }

    /** Helper mapping a vendor string to it's eqivalence class ID.
     *
     * \li Return the vendor strings eqivalence class ID stored in _vendorMatch.
     * \li If not found, assign and return the eqivalence class ID of the lowercased string.
     * \li If not found, assign and return a new ID (look into the predefined VendorGroupMap (id>0),
     *     otherwise create a new ID (<0)).
     */
    unsigned vendorMatchId( IdString vendor ) const;

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };

  unsigned VendorAttr::Impl::vendorMatchId( IdString vendor ) const
  {
    VendorMatchEntry & ent { _vendorMatch[vendor] };
    if ( ! ent )
    {
      IdString lcvendor { str::toLower( vendor.asString() ) };
      VendorMatchEntry & lcent( _vendorMatch[lcvendor] );
      if ( ! lcent )
      {
        // Cache miss - check whether it belongs to a vendor group.
        // Otherwise assign a new class ID to it.
        unsigned myid = 0;

        // bnc#812608: no prefix compare in opensuse namespace
        if ( str::hasPrefix( lcvendor.c_str(), "opensuse" ) )
        {
          if ( auto it = _vendorGroupMap.find( lcvendor.c_str() ); it != _vendorGroupMap.end() )
            myid = it->second;
        }
        else
        {
          // Compare this entry with the global vendor map.
          // Reversed to get the longest prefix.
          for ( VendorGroupMap::const_reverse_iterator it = _vendorGroupMap.rbegin(); it != _vendorGroupMap.rend(); ++it )
          {
            if ( str::hasPrefix( lcvendor.c_str(), it->first ) ) {
              myid = it->second;
              break; // found
            }
          }
        }

        if ( ! myid )
          myid = --_nextId; // get a new class ID

        ent = lcent = myid; // remember the new DI
      }
      else
        ent = lcent; // take the ID from the lowercased vendor string
    }
    return ent;
  }

  void VendorAttr::Impl::addVendorList( VendorList && vendorList_r )
  {
    // Will add a new equivalence group unless we merge with existing groups.
    unsigned targetId = _vendorGroupId + 1;

    // (!) Convert the group strings in place to lowercase before adding/checking.
    // Extend/join already existing groups if they are referenced.
    for ( std::string & vendor : vendorList_r )
    {
      vendor = str::toLower( std::move(vendor) );

      if ( _vendorGroupMap.count( vendor ) )
      {
        unsigned joinId = _vendorGroupMap[vendor];
        if ( targetId == _vendorGroupId + 1 ) {
          targetId = joinId;	// will extend the existing group
        }
        else if ( targetId != joinId ) {
          // yet another existing group -> join it into the target group
          for ( auto & el : _vendorGroupMap ) {
            if ( el.second == joinId )
              el.second = targetId;
          }
        }
        vendor.clear();	// no need to add it later
      }
    }

    // Now add the new entries
    for ( std::string & vendor : vendorList_r ) {
      if ( ! vendor.empty() )
        _vendorGroupMap[vendor] = targetId;
    }

    if ( targetId == _vendorGroupId + 1 )
      ++_vendorGroupId;

    // invalidate any match cache
    vendorMatchIdReset();
  }

  /** \relates VendorAttr::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const VendorAttr::Impl & obj )
  {
    str << "Equivalent vendors:";
    for( const auto & p : obj._vendorGroupMap ) {
      str << endl << "   [" << p.second << "] " << p.first;
    }
    return str;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : VendorAttr
  //
  ///////////////////////////////////////////////////////////////////

  const VendorAttr & VendorAttr::instance()
  {
    Target_Ptr trg { getZYpp()->getTarget() };
    return trg ? trg->vendorAttr() : noTargetInstance();
  }

  VendorAttr & VendorAttr::noTargetInstance()
  {
    static VendorAttr _val { ZConfig::instance().vendorPath() };
    return _val;
  }

  VendorAttr::VendorAttr()
  : _pimpl( new Impl )
  {
    MIL << "Initial: " << *this << endl;
  }

  VendorAttr::VendorAttr( const Pathname & initial_r )
  : _pimpl( new Impl )
  {
    addVendorDirectory( initial_r );
    MIL << "Initial " << initial_r << ": " << *this << endl;
  }

  VendorAttr::~VendorAttr()
  {}

  bool VendorAttr::addVendorDirectory( const Pathname & dirname_r )
  {
    if ( PathInfo pi { dirname_r }; ! pi.isDir() ) {
      MIL << "Not a directory " << pi << endl;
      return false;
    }

    filesystem::dirForEach( dirname_r, filesystem::matchNoDots(),
                            [this]( const Pathname & dir_r, const std::string & str_r )->bool
                            {
                              this->addVendorFile( dir_r/str_r );
                              return true;
                            }
    );
    return true;
  }

  bool VendorAttr::addVendorFile( const Pathname & filename_r )
  {
    if ( PathInfo pi { filename_r }; ! pi.isFile() ) {
      MIL << "Not a file " << pi << endl;
      return false;
    }

    parser::IniDict dict { InputStream(filename_r) };
    for ( const auto & el : dict.entries("main") )
    {
      if ( el.first == "vendors" )
      {
        VendorList tmp;
        strv::split( el.second, ",", strv::Trim::trim,
                     [&tmp]( std::string_view word ) {
                       if ( ! word.empty() )
                         tmp.push_back( std::string(word) );
                     } );
        _addVendorList( std::move(tmp) );
        break;
      }
    }
    return true;
  }

  void VendorAttr::_addVendorList( VendorList && vendorList_r )
  { _pimpl->addVendorList( std::move(vendorList_r) ); }

  unsigned VendorAttr::foreachVendorList( std::function<bool(VendorList)> fnc_r ) const
  { return _pimpl->foreachVendorList( std::move(fnc_r) ); }

#if LEGACY(1722)
  bool VendorAttr::addVendorDirectory( const Pathname & dirname ) const
  { return const_cast<VendorAttr*>(this)->addVendorDirectory( dirname ); }

  bool VendorAttr::addVendorFile( const Pathname & filename ) const
  { return const_cast<VendorAttr*>(this)->addVendorFile( filename ); }

  void VendorAttr::_addVendorList( std::vector<std::string> & vendorList_r ) const
  { return const_cast<VendorAttr*>(this)->_addVendorList( VendorList( vendorList_r.begin(), vendorList_r.end() ) ); }

  void VendorAttr::_addVendorList( std::vector<IdString> && list_r )
  {
    VendorList tmp;
    for ( const auto & el : list_r )
      tmp.push_back( std::string(el) );
    _addVendorList( std::move(tmp) );
  }
#endif
  //////////////////////////////////////////////////////////////////
  // vendor equivalence:
  //////////////////////////////////////////////////////////////////

  bool VendorAttr::equivalent( IdString lVendor, IdString rVendor ) const
  { return _pimpl->equivalent( lVendor, rVendor );}

  bool VendorAttr::equivalent( const Vendor & lVendor, const Vendor & rVendor ) const
  { return _pimpl->equivalent( IdString( lVendor ), IdString( rVendor ) ); }

  bool VendorAttr::equivalent( sat::Solvable lVendor, sat::Solvable rVendor ) const
  { return _pimpl->equivalent( lVendor.vendor(), rVendor.vendor() ); }

  bool VendorAttr::equivalent( const PoolItem & lVendor, const PoolItem & rVendor ) const
  { return _pimpl->equivalent( lVendor.satSolvable().vendor(), rVendor.satSolvable().vendor() ); }


  bool VendorAttr::relaxedEquivalent( IdString lVendor, IdString rVendor ) const
  { return _pimpl->relaxedEquivalent( lVendor, rVendor );}

  bool VendorAttr::relaxedEquivalent( const Vendor & lVendor, const Vendor & rVendor ) const
  { return _pimpl->relaxedEquivalent( IdString( lVendor ), IdString( rVendor ) ); }

  bool VendorAttr::relaxedEquivalent( sat::Solvable lVendor, sat::Solvable rVendor ) const
  { return _pimpl->relaxedEquivalent( lVendor.vendor(), rVendor.vendor() ); }

  bool VendorAttr::relaxedEquivalent( const PoolItem & lVendor, const PoolItem & rVendor ) const
  { return _pimpl->relaxedEquivalent( lVendor.satSolvable().vendor(), rVendor.satSolvable().vendor() ); }

  //////////////////////////////////////////////////////////////////

  std::ostream & operator<<( std::ostream & str, const VendorAttr & obj )
  { return str << *obj._pimpl; }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

