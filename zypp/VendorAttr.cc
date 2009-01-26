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
    VendorMap _matchMap;
    unsigned int vendorGroupCounter;

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
      try
      {
	  Target_Ptr trg( getZYpp()->target() );
	  if ( trg )
	      vendorPath = trg->root() / vendorPath;
      }
      catch ( ... )
      {
	  // noop: Someone decided to let target() throw if the ptr is NULL ;(
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

      // Take care 'opensuse build service' gets it's own class.
      VendorMap::const_iterator obsit( _vendorMap.find("opensuse build service") );
      if ( obsit == _vendorMap.end() )
      {
        _vendorMap["opensuse build service"] = ++vendorGroupCounter;
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
      parser::IniDict dict;

      if ( PathInfo(dirname).isExist())
      {
          InputStream is(dirname);
          dict.read(is);
      }
      else
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


  bool VendorAttr::equivalent( const Vendor & lVendor, const Vendor & rVendor ) const
  {
      unsigned int lhsID = 0;
      unsigned int rhsID = 0;
      Vendor lhs = str::toLower (lVendor);
      Vendor rhs = str::toLower (rVendor);

      if ( lhs == rhs )
	  return true;

      // NOTE: iterate reverse to get find longest prefix first!

      if (_matchMap.find(lhs) != _matchMap.end()) {
	  lhsID = _matchMap[lhs];
      } else {
	  // compare this entry with the vendor map
	  for (VendorMap::reverse_iterator it = _vendorMap.rbegin();
	       it != _vendorMap.rend();
	       ++it) {
	      if (lhs.substr (0, it->first.size())  == it->first) {
		  lhsID = it->second;
		  _matchMap[lhs] = lhsID;
		  break; // exit for
	      }
	  }
      }

      if (_matchMap.find(rhs) != _matchMap.end()) {
	  rhsID = _matchMap[rhs];
      } else {
	  // compare this entry with the vendor map
	  for (VendorMap::reverse_iterator it = _vendorMap.rbegin();
	       it != _vendorMap.rend();
	       ++it) {
	      if (rhs.substr (0, it->first.size())  == it->first) {
		  rhsID = it->second;
		  _matchMap[rhs] = rhsID;
		  break; // exit for
	      }
	  }
      }
      return( lhsID && rhsID && lhsID == rhsID  );
  }

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

