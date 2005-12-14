/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ResolverInfoMisc.h
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

#ifndef _ResolverInfoMisc_h
#define _ResolverInfoMisc_h

#include <string>
#include <zypp/solver/detail/ResolverInfoMiscPtr.h>
#include <zypp/solver/detail/ResolverInfoContainer.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ResolverInfoMisc

class ResolverInfoMisc : public ResolverInfoContainer {

    REP_BODY(ResolverInfoMisc);

  private:

    std::string _msg;
    std::string _action;
    std::string _trigger;

  public:

    ResolverInfoMisc (constResItemPtr resItem, int priority, const std::string & msg);
    virtual ~ResolverInfoMisc();

    // ---------------------------------- I/O

    static std::string toString (const ResolverInfoMisc & context);
    virtual std::ostream & dumpOn(std::ostream & str ) const;
    friend std::ostream& operator<<(std::ostream&, const ResolverInfoMisc & context);
    std::string asString (void ) const;

    // ---------------------------------- accessors

    // ---------------------------------- methods

    virtual bool merge (ResolverInfoPtr to_be_merged);
    virtual ResolverInfoPtr copy (void) const;

    void addAction (const std::string & action_msg);
    void addTrigger (const std::string & trigger_msg);

};
 
///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////
#endif // _ResolverInfoMisc_h
 
