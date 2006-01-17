/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
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

#include <assert.h>
#include "zypp/solver/temporary/ResItem.h"
#include "zypp/ResObject.h"
#include "zypp/Package.h"
#include "zypp/Selection.h"
#include "zypp/Pattern.h"
#include "zypp/Patch.h"
#include "zypp/Message.h"
#include "zypp/Script.h"
#include "zypp/Product.h"
#include "zypp/detail/PackageImpl.h"
#include "zypp/detail/SelectionImpl.h"
#include "zypp/detail/PatternImpl.h"
#include "zypp/detail/PatchImpl.h"
#include "zypp/detail/MessageImpl.h"
#include "zypp/detail/ScriptImpl.h"
#include "zypp/detail/ProductImpl.h"
#include "zypp/base/String.h"


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


      IMPL_PTR_TYPE(ResItem);

      //---------------------------------------------------------------------------

      string
      capSetToString (const CapSet & dl)
      {
	  string res("[");
	  for (CapSet::const_iterator iter = dl.begin(); iter != dl.end(); iter++) {
	      if (iter != dl.begin()) res += ", ";
	      res += (*iter).asString();
	  }
	  return res + "]";
      }


      string
      ResItem::asString ( bool full ) const
      {
	  return toString (*this, full);
      }


      string
      ResItem::toString ( const ResItem & resItem, bool full )
      {
	  string res;

	  if (full
	      || (resItem.kind() != ResTraits<zypp::Package>::kind))	// only show non-package kinds
	  {
	      res += resItem.kind().asString();
	      res += ":";
	  }

	  res += resItem.name();

	  string ed = resItem.edition().asString();
	  if (!ed.empty() &&
	      ed != "EDITION-UNSPEC")
	  {
	      res += "-";
	      res += ed;
	  }

	  if (resItem.arch() != Arch_noarch) {
	      res += ".";
	      res += resItem.arch().asString();
	  }

	  if (!resItem.channel()->system()) {
      	res += "[";
      	res += (resItem.channel() == NULL) ? "(channel?)" : resItem.channel()->name();
      	res += "]";
	  }
	  if (!full) return res;

	  if (resItem.isInstalled()) res += "<installed>";
	  if (resItem.local()) res += "<local>";

	  res += "FileSize ";
	  res += str::numstring (resItem.fileSize());
	  res += ", InstalledSize ";
	  res += str::numstring (resItem.installedSize());

	  if (!resItem.requires().empty()) {
      	res += ", Requires: ";
      	res += capSetToString(resItem.requires());
	  }

	  if (!resItem.provides().empty()) {
      	res += ", Provides: ";
      	res += capSetToString(resItem.provides());
	  }
	  if (!resItem.conflicts().empty()) {
      	res += ", Conflicts: ";
      	res += capSetToString(resItem.conflicts());
	  }
	  if (!resItem.obsoletes().empty()) {
      	res += ", Obsoletes: ";
      	res += capSetToString(resItem.obsoletes());
	  }

	  if (!resItem.suggests().empty()) {
      	res += ", Suggests: ";
      	res += capSetToString(resItem.suggests());
	  }
	  if (!resItem.recommends().empty()) {
      	res += ", Recommends: ";
      	res += capSetToString(resItem.recommends());
	  }
	  if (!resItem.freshens().empty()) {
      	res += ", Freshens: ";
      	res += capSetToString(resItem.freshens());
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

      ResItem::ResItem (const Resolvable::Kind & kind, const string & name, const Edition & edition, const Arch & arch)
	  : _channel (false)
	  , _installed (false)
	  , _local (false)
	  , _locked (false)
	  , _file_size (0)
	  , _installed_size (0)

      {
//	  Edition     _edition( version, release, zypp::str::numstring(epoch) );

	  // create the ResObject
	  if (kind == ResTraits<zypp::Package>::kind)
	  {
	      shared_ptr<zypp::detail::PackageImpl> pkgImpl;
	      zypp::Package::Ptr pkg( zypp::detail::makeResolvableAndImpl( NVRAD(name, edition, arch),
				                                           pkgImpl ) );
	      _resObject = pkg;
	  } else if (kind == ResTraits<zypp::Selection>::kind)
	  {
	      shared_ptr<zypp::detail::SelectionImpl> selImpl;
	      zypp::Selection::Ptr sel( zypp::detail::makeResolvableAndImpl( NVRAD(name, edition, arch),
				                                             selImpl ) );
	      _resObject = sel;
	  } else if (kind == ResTraits<zypp::Pattern>::kind)
	  {
	      shared_ptr<zypp::detail::PatternImpl> patternImpl;
	      zypp::Pattern::Ptr pattern( zypp::detail::makeResolvableAndImpl( NVRAD(name, edition, arch),
				                                             patternImpl ) );
	      _resObject = pattern;
	  } else if (kind == ResTraits<zypp::Product>::kind)
	  {
//              shared_ptr<zypp::detail::ProductImpl> proImpl;
//              zypp::Product::Ptr pro( zypp::detail::makeResolvableAndImpl( NVRAD(name, edition, arch),
//                                                                           proImpl ) );
//              _resObject = pro;
	  } else if (kind == ResTraits<zypp::Patch>::kind)
	  {
	      shared_ptr<zypp::detail::PatchImpl> patchImpl;
	      zypp::Patch::Ptr patch( zypp::detail::makeResolvableAndImpl( NVRAD(name, edition, arch),
				                                           patchImpl ) );
	      _resObject = patch;
	  } else if (kind == ResTraits<zypp::Script>::kind)
	  {
	      shared_ptr<zypp::detail::ScriptImpl> scriptImpl;
	      zypp::Script::Ptr script( zypp::detail::makeResolvableAndImpl( NVRAD(name, edition, arch),
				                                             scriptImpl ) );
	      _resObject = script;
	  } else if (kind == ResTraits<zypp::Message>::kind)
	  {
	      shared_ptr<zypp::detail::MessageImpl> messageImpl;
	      zypp::Message::Ptr message( zypp::detail::makeResolvableAndImpl( NVRAD(name, edition, arch),
				                                               messageImpl ) );
	      _resObject = message;
	  }

      }

      ResItem::ResItem(const ResObject::Ptr & resObject)
	  : _channel (false)
	  , _installed (false)
	  , _local (false)
	  , _locked (false)
	  , _file_size (0)
	  , _installed_size (0)
      {
	  assert (resObject);
	  _resObject = resObject;
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

      bool
      ResItem::equals(ResItem_constPtr item) const {
	  return ((kind() == item->kind())
		&& (name() == item->name())
      	&& Edition::compare( edition(), item->edition()) == 0);
      }

      bool ResItem::equals(const  Resolvable::Kind & compKind,
			   const string & compName,
		      const Edition & compEdition) const {
	  return ((compKind == kind())
		&& (compName == name())
      	&& Edition::compare( compEdition, edition()) == 0);
      }

      int  ResItem::compare (ResItem_constPtr res1, ResItem_constPtr res2) {
	  int rc = 0;

	  const string name1 = res1->name();
	  const string name2 = res2->name();
	  if (! (name1.empty() && name2.empty()))
	  {
	      rc = name1.compare (name2);
	  }
	  if (rc) return rc;

	  rc = Edition::compare (res1->edition(),
				 res2->edition());

	  return rc;
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
