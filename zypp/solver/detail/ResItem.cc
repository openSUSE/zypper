/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ResItem.cc
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * Definition of 'resItem'
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <y2util/stringutil.h>

#include <zypp/solver/detail/ResItem.h>
#include <zypp/ResObject.h>
#include <zypp/detail/ResObjectImplIf.h>
#include <zypp/detail/ResObjectFactory.h>
#include <zypp/Package.h>
#include <zypp/detail/PackageImpl.h>
#include <zypp/base/String.h>


/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////
      
      using namespace std;
      
      
      IMPL_DERIVED_POINTER(ResItem,Spec);
      
      //---------------------------------------------------------------------------
      
      string
      ResItem::asString ( bool full ) const
      {
          return toString (*this, full);
      }
      
      
      string
      ResItem::toString ( const ResItem & resItem, bool full )
      {
          string res;
      
          res += Spec::toString(resItem);
          if (!resItem.channel()->system()) {
      	res += "[";
      	res += (resItem.channel() == NULL) ? "(channel?)" : resItem.channel()->name();
      	res += "]";
          }
          if (!full) return res;
      
          if (resItem.isInstalled()) res += "<installed>";
          if (resItem.local()) res += "<local>";
      
          res += "FileSize ";
          res += stringutil::numstring (resItem.fileSize());
          res += ", InstalledSize ";
          res += stringutil::numstring (resItem.installedSize());
      
          if (!resItem.requires().empty()) {
      	res += ", Requires: ";
      	res += Dependency::toString(resItem.requires());
          }
      
          if (!resItem.provides().empty()) {
      	res += ", Provides: ";
      	res += Dependency::toString(resItem.provides());
          }
          if (!resItem.conflicts().empty()) {
      	res += ", Conflicts: ";
      	res += Dependency::toString(resItem.conflicts());
          }
          if (!resItem.obsoletes().empty()) {
      	res += ", Obsoletes: ";
      	res += Dependency::toString(resItem.obsoletes());
          }
      
          if (!resItem.suggests().empty()) {
      	res += ", Suggests: ";
      	res += Dependency::toString(resItem.suggests());
          }
          if (!resItem.recommends().empty()) {
      	res += ", Recommends: ";
      	res += Dependency::toString(resItem.recommends());
          }
          if (!resItem.freshens().empty()) {
      	res += ", Freshens: ";
      	res += Dependency::toString(resItem.freshens());
          }
          return res;
      }
      
      
      string
      ResItem::toString ( const CResItemList & rl, bool full )
      {
          string res("[");
          for (CResItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
      	if (iter != rl.begin()) res += ", ";
      	res += (*iter)->asString(full);
          }
          return res + "]";
      }
      
      
      ostream &
      ResItem::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream& os, const ResItem& edition)
      {
          return os << edition.asString();
      }
      
      //---------------------------------------------------------------------------
      
      ResItem::ResItem (const Resolvable::Kind & kind, const string & name, int epoch, const string & version, const string & release, const zypp::Arch & arch)
          :Spec (kind, name, epoch, version, release, arch)
          , _channel (false)
          , _installed (false)
          , _local (false)
          , _locked (false)
          , _file_size (0)
          , _installed_size (0)
      
      {
          zypp::Edition     _edition( version, release, zypp::str::numstring(epoch) );

          // create the ResObject
          shared_ptr<zypp::detail::PackageImpl> pkgImpl;
          zypp::Package::Ptr pkg( zypp::detail::makeResolvableAndImpl( name, _edition, arch,
                                                   pkgImpl ) );
          _resObject = pkg;

//          shared_ptr<zypp::detail::ResObjectImplIf> resObjectImpl;
//          ResObject::Ptr resObject( zypp::detail::makeResolvableAndImpl( _name, _edition, _arch,
//                                                   resObjectImpl ) );
//          
//        _resObject = resObject;
          
          
      }

      ResItem::ResItem(const ResObject::Ptr & resObject)
          :Spec (ResTraits<Package>::kind, "")
          ,_channel (false)
          , _installed (false)
          , _local (false)
          , _locked (false)
          , _file_size (0)
          , _installed_size (0)
      {
          _resObject = resObject;
          if (_resObject != NULL)
          {
              setVersion(resObject->edition().version());
              setArch(resObject->arch().asString());
              setEpoch(resObject->edition().epoch());
              setRelease(resObject->edition().release());
              setKind(resObject->kind());
              setName(resObject->name());    
          }
      }
      
      ResItem::~ResItem()
      {
      }
      
      //---------------------------------------------------------------------------
      
      bool
      ResItem::isInstalled () const
      {
          if (_channel != NULL
      	&& _channel->system()) {
      	return true;
          }
          return false;
      }
      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////
