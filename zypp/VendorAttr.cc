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
#include <zypp/parser/IniDict.h>

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
    /** bsc#1030686: The legacy default equivalence of 'suse' and 'opensuse'
     * has been removed. Unless they are mentioned in a custom rule, create
     * separate classes for them.
     */
    void legacySetup()
    {
      if ( _vendorMap.find("suse") == _vendorMap.end() )
	_vendorMap["suse"] = ++vendorGroupCounter;

      if ( _vendorMap.find("opensuse") == _vendorMap.end() )
	_vendorMap["opensuse"] = ++vendorGroupCounter;
    }

  public:
    /** Add a new equivalent vendor set. */
    void addVendorList( VendorList && vendorList_r );

    /** Return whether two vendor strings should be treated as equivalent.*/
    bool equivalent( IdString lVendor, IdString rVendor ) const
    { return lVendor == rVendor || vendorMatchId( lVendor ) == vendorMatchId( rVendor ); }

  private:
    using VendorMap = std::map<std::string,unsigned>;
    mutable VendorMap _vendorMap;
    unsigned vendorGroupCounter = 1;

  private:
    typedef DefaultIntegral<int,0>				VendorMatchEntry;
    typedef std::unordered_map<IdString, VendorMatchEntry>	VendorMatch;
    mutable int         _nextId = -1;
    mutable VendorMatch _vendorMatch;

    /** Reset match cache if global VendorMap was changed. */
    void vendorMatchIdReset()
    {
      _nextId = -1;
      _vendorMatch.clear();
    }

    /**
     * Helper mapping vendor string to eqivalence class ID.
     *
     * \li Return the vendor strings eqivalence class ID stored in _vendorMatch.
     * \li If not found, assign and return the eqivalence class ID of the lowercased string.
     * \li If not found, assign and return a new ID (look into the predefined VendorMap (id>0),
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
    VendorMatchEntry & ent( _vendorMatch[vendor] );
    if ( ! ent )
    {
      IdString lcvendor( str::toLower( vendor.asString() ) );
      VendorMatchEntry & lcent( _vendorMatch[lcvendor] );
      if ( ! lcent )
      {
	unsigned myid = 0;
	// bnc#812608: no prefix compare in opensuse namespace
	static const IdString openSUSE( "opensuse" );
	if ( lcvendor == openSUSE || ! str::hasPrefix( lcvendor.c_str(), openSUSE.c_str() ) )
	{
	  // Compare this entry with the global vendor map.
	  // Reversed to get the longest prefix.
	  for ( VendorMap::reverse_iterator it = _vendorMap.rbegin(); it != _vendorMap.rend(); ++it )
	  {
	    if ( str::hasPrefix( lcvendor.c_str(), it->first ) )
	    {
	      myid = it->second;
	      break; // found
	    }
	  }
	}
	if ( ! myid )
	{
	  myid = --_nextId; // get a new class ID
	}
	ent = lcent = myid; // remember the new DI
      }
      else
      {
	ent = lcent; // take the ID from the lowercased vendor string
      }
    }
    return ent;
  }

  void VendorAttr::Impl::addVendorList( VendorList && vendorList_rr )
  {
#warning REIMPLEMENT
    std::vector<std::string> vendorList_r;
    for ( const auto & el : vendorList_rr ) {
      vendorList_r.push_back( el.asString() );
    }

    unsigned int nextId = vendorGroupCounter + 1;
    // convert to lowercase and check if a vendor is already defined
    // in an existing group.

    for_( it, vendorList_r.begin(), vendorList_r.end() )
    {
      *it = str::toLower( *it );
      if (_vendorMap.find(*it) != _vendorMap.end())
      {
        if (nextId != vendorGroupCounter + 1 &&
            nextId != _vendorMap[*it])
        {
	  // We have at least 3 groups which has to be mixed --> mix the third group to the first
          unsigned int moveID = _vendorMap[*it];
          for_( itMap, _vendorMap.begin(), _vendorMap.end() )
          {
            if (itMap->second == moveID)
              itMap->second = nextId;
          }
        }
        else
        {
          nextId = _vendorMap[*it];
          WAR << "Vendor " << *it << " is already used in another vendor group. --> mixing these groups" << endl;
        }
      }
    }
	// add new entries
    for_( it, vendorList_r.begin(), vendorList_r.end() )
    {
      _vendorMap[*it] = nextId;
    }

    if (nextId == vendorGroupCounter + 1)
      ++vendorGroupCounter;

    // invalidate any match cache
    vendorMatchIdReset();
  }

  /** \relates VendorAttr::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const VendorAttr::Impl & obj )
  {
    str << "Equivalent vendors:";
    for( const auto & p : obj._vendorMap ) {
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
    _pimpl->legacySetup();
    MIL << "Initial: " << *this << endl;
  }

  VendorAttr::VendorAttr( const Pathname & initial_r )
  : _pimpl( new Impl )
  {
    addVendorDirectory( initial_r );
    _pimpl->legacySetup();
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
			 tmp.push_back( IdString( word ) );
		     } );
	_addVendorList( std::move(tmp) );
	break;
      }
    }
    return true;
  }

  void VendorAttr::_addVendorList( VendorList && vendorList_r )
  { _pimpl->addVendorList( std::move(vendorList_r) ); }

#if LEGACY(1722)
  bool VendorAttr::addVendorDirectory( const Pathname & dirname ) const
  { return const_cast<VendorAttr*>(this)->addVendorDirectory( dirname ); }

  bool VendorAttr::addVendorFile( const Pathname & filename ) const
  { return const_cast<VendorAttr*>(this)->addVendorFile( filename ); }

  void VendorAttr::_addVendorList( std::vector<std::string> & vendorList_r ) const
  { return const_cast<VendorAttr*>(this)->_addVendorList( VendorList( vendorList_r.begin(), vendorList_r.end() ) ); }
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

  //////////////////////////////////////////////////////////////////

  std::ostream & operator<<( std::ostream & str, const VendorAttr & obj )
  { return str << *obj._pimpl; }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

