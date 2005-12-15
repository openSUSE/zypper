/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ResolverInfoContainer.h
 *
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

#ifndef _ResolverInfoContainer_h
#define _ResolverInfoContainer_h

#include <iosfwd>
#include <list>
#include <map>
#include <string.h>
#include <zypp/solver/detail/ResolverInfoContainerPtr.h>
#include <zypp/solver/detail/ResolverInfo.h>

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

      //////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : ResolverInfoContainer
      
      class ResolverInfoContainer : public ResolverInfo {
      
          REP_BODY(ResolverInfoContainer);
      
        private:
      
          CResItemList _resItem_list;
      
        protected:
      
          ResolverInfoContainer (ResolverInfoType type, constResItemPtr resItem, int priority, constResItemPtr child = NULL);
      
        public:
          virtual ~ResolverInfoContainer();
      
          void copy (constResolverInfoContainerPtr from);
      
          // ---------------------------------- I/O
      
          static std::string toString (const ResolverInfoContainer & context);
          virtual std::ostream & dumpOn(std::ostream & str ) const;
          friend std::ostream& operator<<(std::ostream&, const ResolverInfoContainer & context);
          std::string asString (void ) const;
      
          // ---------------------------------- accessors
      
          CResItemList resItems (void) const { return _resItem_list; }
      
          // ---------------------------------- methods
      
          virtual bool merge (ResolverInfoContainerPtr to_be_merged);
          virtual ResolverInfoPtr copy (void) const;
      
          std::string resItemsToString (bool names_only) const;
      
          bool mentions (constResItemPtr resItem) const;
          void addRelatedResItem (constResItemPtr resItem);
          void addRelatedResItemList (const CResItemList & resItems);
      
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
#endif // _ResolverInfoContainer_h
 
