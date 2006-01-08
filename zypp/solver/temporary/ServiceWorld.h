/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ServiceWorld.h
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

#ifndef ZYPP_SOLVER_TEMPORARY_SERVICEWORLD_H
#define ZYPP_SOLVER_TEMPORARY_SERVICEWORLD_H

#include <iosfwd>
#include <string>

#include "zypp/solver/temporary/ServiceWorldPtr.h"
#include "zypp/solver/temporary/StoreWorld.h"
#include "zypp/solver/temporary/Channel.h"


/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

typedef bool (*ServiceWorldAssembleFn) (ServiceWorld_Ptr service, void *error);	// GError **error

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ServiceWorld

class ServiceWorld : public StoreWorld {
    

  private:

    std::string _url;
    std::string _name;
    std::string _unique_id;

    bool _is_sticky;		// if true, can't be unmounted
    bool _is_invisible;		// ... to users
    bool _is_unsaved;		// Never save into the services.xml file
    bool _is_singleton;		// only one such service at a time.  FIXME: broken

    ServiceWorldAssembleFn _assemble_fn;

  public:

    ServiceWorld ();
    virtual ~ServiceWorld();

    // ---------------------------------- I/O

    static std::string toString (const ServiceWorld & section);

    virtual std::ostream & dumpOn(std::ostream & str ) const;

    friend std::ostream& operator<<(std::ostream&, const ServiceWorld & section);

    std::string asString (void ) const;

    // ---------------------------------- accessors

    std::string url () const { return _url; }
    std::string name () const { return _name; }
    void setName (const std::string & name) { _name = name; }
    std::string unique_id () const { return _unique_id; }

    // ---------------------------------- methods

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


#endif // ZYPP_SOLVER_TEMPORARY_SERVICEWORLD_H
