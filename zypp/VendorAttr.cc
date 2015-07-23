/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/*
  File:       VendorAttr.cc

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose: Manage vendor attributes

/-*/

#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include <vector>

#include "zypp/base/LogTools.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/String.h"

#include "zypp/PathInfo.h"
#include "zypp/VendorAttr.h"
#include "zypp/ZYppFactory.h"

#include "zypp/ZConfig.h"
#include "zypp/PathInfo.h"
#include "zypp/parser/IniDict.h"

using namespace std;

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::VendorAttr"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////

    typedef map<Vendor,unsigned int> VendorMap;
    VendorMap _vendorMap;
    unsigned int vendorGroupCounter;

    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////
    typedef DefaultIntegral<int,0>				VendorMatchEntry;
    typedef std::unordered_map<IdString, VendorMatchEntry>	VendorMatch;
    int         _nextId = -1;
    VendorMatch _vendorMatch;

    /** Reset match cache if global VendorMap was changed. */
    inline void vendorMatchIdReset()
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
    inline unsigned vendorMatchId( IdString vendor )
    {
      VendorMatchEntry & ent( _vendorMatch[vendor] );
      if ( ! ent )
      {
        IdString lcvendor( str::toLower( vendor.asString() ) );
        VendorMatchEntry & lcent( _vendorMatch[lcvendor] );
        if ( ! lcent )
        {
          unsigned myid = 0;
	  // bnc#812608: no pefix compare in opensuse namespace
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
    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  const VendorAttr & VendorAttr::instance()
  {
      static VendorAttr _val;
      return _val;
  }

  VendorAttr::VendorAttr ()
  {
      vendorGroupCounter = 1;
      Pathname vendorPath (ZConfig::instance().vendorPath());
      {
	Target_Ptr trg( getZYpp()->getTarget() );
	if ( trg )
	  vendorPath = trg->root() / vendorPath;
      }
      // creating entries
      addVendorDirectory (vendorPath);

      // Checking if suse,opensuse has been defined, else create entries:
      // - if both are defined we leve them as thay are.
      // - if only one of them is defined, we add the other to the same group.
      // - if both are undefined they make up a new group
      VendorMap::const_iterator suseit( _vendorMap.find("suse") );
      VendorMap::const_iterator opensuseit( _vendorMap.find("opensuse") );
      if ( suseit == _vendorMap.end() )
      {
        if ( opensuseit == _vendorMap.end() )
        {
          // both are undefined
          _vendorMap["suse"] = _vendorMap["opensuse"] = ++vendorGroupCounter;
        }
        else
        {
          // add suse to opensuse
          _vendorMap["suse"] = opensuseit->second;
        }
      }
      else if ( opensuseit == _vendorMap.end() )
      {
        // add opensuse to suse
        _vendorMap["opensuse"] = suseit->second;
      }

      MIL << *this << endl;
  }

  void VendorAttr::_addVendorList( VendorList & vendorList_r ) const
  {
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

  bool VendorAttr::addVendorFile( const Pathname & filename ) const
  {
      parser::IniDict dict;

      if ( PathInfo(filename).isExist())
      {
          InputStream is(filename);
          dict.read(is);
      }
      else
      {
          MIL << filename << " not found." << endl;
          return false;
      }

      for ( parser::IniDict::section_const_iterator sit = dict.sectionsBegin();
	    sit != dict.sectionsEnd();
	    ++sit )
      {
          string section(*sit);
          //MIL << section << endl;
          for ( parser::IniDict::entry_const_iterator it = dict.entriesBegin(*sit);
                it != dict.entriesEnd(*sit);
                ++it )
          {
	      string entry(it->first);
	      string value(it->second);
	      if ( section == "main" )
	      {
		  if ( entry == "vendors" )
		  {
		      VendorList vendorlist;
		      str::split( value, back_inserter(vendorlist), "," );
		      _addVendorList (vendorlist);
		      break;
		  }
	      }
	  }
      }

      return true;
  }

  bool VendorAttr::addVendorDirectory( const Pathname & dirname ) const
  {
      if ( ! PathInfo(dirname).isExist() )
      {
          MIL << dirname << " not found." << endl;
          return false;
      }

      list<Pathname> filenames;

      filesystem::readdir( filenames,
			   dirname, false );
      for (list<Pathname>::iterator it = filenames.begin();
	   it != filenames.end(); ++it) {
	  MIL << "Adding file " << *it << endl;
	  addVendorFile( *it );
      }
      return true;
  }

  //////////////////////////////////////////////////////////////////
  // vendor equivalence:
  //////////////////////////////////////////////////////////////////

  bool VendorAttr::equivalent( IdString lVendor, IdString rVendor ) const
  {
    if ( lVendor == rVendor )
      return true;
    return vendorMatchId( lVendor ) == vendorMatchId( rVendor );
  }

  bool VendorAttr::equivalent( const Vendor & lVendor, const Vendor & rVendor ) const
  { return equivalent( IdString( lVendor ), IdString( rVendor ) );
  }

  bool VendorAttr::equivalent( sat::Solvable lVendor, sat::Solvable rVendor ) const
  { return equivalent( lVendor.vendor(), rVendor.vendor() ); }

  bool VendorAttr::equivalent( const PoolItem & lVendor, const PoolItem & rVendor ) const
  { return equivalent( lVendor.satSolvable().vendor(), rVendor.satSolvable().vendor() ); }

  //////////////////////////////////////////////////////////////////

  std::ostream & operator<<( std::ostream & str, const VendorAttr & /*obj*/ )
  {
    str << "Equivalent vendors:";
    for_( it, _vendorMap.begin(), _vendorMap.end() )
    {
      str << endl << "   [" << it->second << "] " << it->first;
    }
    return str;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

