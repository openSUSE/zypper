/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResItem.h
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
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

#ifndef ZYPP_SOLVER_TEMPORARY_RESITEM_H
#define ZYPP_SOLVER_TEMPORARY_RESITEM_H

#include <list>
#include <set>
#include <iosfwd>
#include <string>
#include <sys/types.h>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/solver/temporary/ResItemPtr.h"
#include "zypp/solver/temporary/StoreWorldPtr.h"
#include "zypp/solver/temporary/Channel.h"

#include "zypp/CapSet.h"
#include "zypp/ResObject.h"
#include "zypp/Edition.h"
#include "zypp/Dependencies.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

      typedef std::list<ResItem_Ptr> ResItemList;
      typedef std::list<ResItem_constPtr> CResItemList;

      typedef std::set<ResItem_constPtr> CResItemSet;

      typedef bool (*ResItemFn) (ResItem_Ptr r, void *data);
      typedef bool (*CResItemFn) (ResItem_constPtr r, void *data);
      typedef bool (*ResItemPairFn) (ResItem_constPtr r1, ResItem_constPtr r2, void *data);
      typedef bool (*ResItemAndDepFn) (ResItem_constPtr r, const Capability & dep, void *data);

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : ResItem
      /**
       *
       **/

      class ResItem : public base::ReferenceCounted, private base::NonCopyable {
          

        private:
          Channel_constPtr _channel;

          bool _installed;
          bool _local;
          bool _locked;

          size_t _file_size;
          size_t _installed_size;

        protected:

          // ---------------------------------- accessors

          void setLocal (bool local) { _local = local; }
          ResObject::Ptr _resObject;

        public:

          ResItem(const Resolvable::Kind & kind, const std::string & name, int epoch = Edition::noepoch, const std::string & version = "", const std::string & release = "", const Arch & arch = Arch());

          ResItem(const ResObject::Ptr & resObject);
          ResItem(const XmlNode_Ptr node);

          virtual ~ResItem();

          // ---------------------------------- I/O

          const XmlNode_Ptr asXmlNode (void) const;

          static std::string toString ( const ResItem & res, bool full = false );

          static std::string toString ( const CResItemList & reslist, bool full = false );

          virtual std::ostream & dumpOn( std::ostream & str ) const;

          friend std::ostream& operator<<( std::ostream & str, const ResItem & str);

          std::string asString ( bool full = false ) const;

          // ---------------------------------- accessors

          Channel_constPtr channel() const { return _channel; }
          void setChannel (Channel_constPtr channel) { _channel = channel; }

          bool locked () const { return _locked; }
          void setLocked (bool locked) { _locked = locked; }

          bool isInstalled() const;				// does *not* reflect _installed
          void setInstalled (bool installed) { _installed = installed; }

          bool local() const { return _local; }

          size_t fileSize() const { return _file_size; }
          void setFileSize (size_t file_size) { _file_size = file_size; }

          size_t installedSize() const { return _installed_size; }
          void setInstalledSize (size_t installed_size) { _installed_size = installed_size; }

          const CapSet & requires() const { return _resObject->deps().requires(); }
          const CapSet & provides() const { return _resObject->deps().provides(); }
          const CapSet & conflicts() const { return _resObject->deps().conflicts(); }
          const CapSet & obsoletes() const { return _resObject->deps().obsoletes(); }
          const CapSet & suggests() const { return _resObject->deps().suggests(); }
          const CapSet & recommends() const { return _resObject->deps().recommends(); }
          const CapSet & freshens() const { return _resObject->deps().freshens(); }

          void setDependencies (const Dependencies & dependencies) { _resObject->setDeps(dependencies); }

          ResObject::constPtr resObject() { return _resObject; }

          // Spec definitions

          const Edition & edition() const { return _resObject->edition(); }
          const std::string & version() const { return edition().version(); }
          const std::string & release() const { return edition().release(); }
          const int epoch() const { return edition().epoch(); }
          bool hasEpoch() const { return edition().epoch() != Edition::noepoch; }
          const Arch & arch() const { return _resObject->arch(); }
          const Resolvable::Kind & kind() const { return _resObject->kind(); }
          const std::string name() const { return _resObject->name(); }
          bool equals(ResItem_constPtr item) const;
          bool equals(const  Resolvable::Kind & kind, const std::string & name,
                      const Edition & edition) const;

          static int compare (ResItem_constPtr res1, ResItem_constPtr res2);

      };

      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

#endif // ZYPP_SOLVER_TEMPORARY_RESITEM_H
