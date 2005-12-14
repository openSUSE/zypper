/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _ResItem_h
#define _ResItem_h

#include <list>
#include <iosfwd>
#include <string.h>
#include <sys/types.h>

#include <zypp/solver/detail/ResItemPtr.h>
#include <zypp/solver/detail/StoreWorldPtr.h>
#include <zypp/solver/detail/Dependency.h>
#include <zypp/solver/detail/Channel.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

typedef std::list<ResItemPtr> ResItemList;
typedef std::list<constResItemPtr> CResItemList;

typedef bool (*ResItemFn) (ResItemPtr r, void *data);
typedef bool (*CResItemFn) (constResItemPtr r, void *data);
typedef bool (*ResItemPairFn) (constResItemPtr r1, constResItemPtr r2, void *data);
typedef bool (*ResItemAndSpecFn) (constResItemPtr r, constSpecPtr spec, void *data);
typedef bool (*ResItemAndDepFn) (constResItemPtr r, constDependencyPtr dep, void *data);

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ResItem
/**
 *
 **/

class ResItem : public Spec {
    REP_BODY(ResItem);

  private:
    constChannelPtr _channel;

    bool _installed;
    bool _local;
    bool _locked;

    size_t _file_size;
    size_t _installed_size;

    CDependencyList _requires;
    CDependencyList _provides;
    CDependencyList _conflicts;
    CDependencyList _obsoletes;

    CDependencyList _suggests;
    CDependencyList _recommends;
    CDependencyList _freshens;

  protected:

    // ---------------------------------- accessors

    void setLocal (bool local) { _local = local; }

  public:

    ResItem(const Kind & kind, const std::string & name, int epoch = -1, const std::string & version = "", const std::string & release = "", const Arch * arch = Arch::Unknown);

    ResItem(const XmlNodePtr node);

    virtual ~ResItem();

    // ---------------------------------- I/O

    const XmlNodePtr asXmlNode (void) const;

    static std::string toString ( const ResItem & res, bool full = false );

    static std::string toString ( const CResItemList & reslist, bool full = false );

    virtual std::ostream & dumpOn( std::ostream & str ) const;

    friend std::ostream& operator<<( std::ostream & str, const ResItem & str);

    std::string asString ( bool full = false ) const;

    // ---------------------------------- accessors

    constChannelPtr channel() const { return _channel; }
    void setChannel (constChannelPtr channel) { _channel = channel; }

    bool locked () const { return _locked; }
    void setLocked (bool locked) { _locked = locked; }

    bool isInstalled() const;				// does *not* reflect _installed
    void setInstalled (bool installed) { _installed = installed; }

    bool local() const { return _local; }

    size_t fileSize() const { return _file_size; }
    void setFileSize (size_t file_size) { _file_size = file_size; }

    size_t installedSize() const { return _installed_size; }
    void setInstalledSize (size_t installed_size) { _installed_size = installed_size; }

    const CDependencyList & requires() const { return _requires; }
    void setRequires (const CDependencyList & requires) { _requires = requires; }

    const CDependencyList & provides() const { return _provides; }
    void setProvides (const CDependencyList & provides) { _provides = provides; }

    const CDependencyList & conflicts() const { return _conflicts; }
    void setConflicts (const CDependencyList & conflicts) { _conflicts = conflicts; }

    const CDependencyList & obsoletes() const { return _obsoletes; }
    void setObsoletes (const CDependencyList & obsoletes) { _obsoletes = obsoletes; }

    const CDependencyList & suggests() const { return _suggests; }
    void setSuggests (const CDependencyList & suggests) { _suggests = suggests; }

    const CDependencyList & recommends() const { return _recommends; }
    void setRecommends (const CDependencyList & recommends) { _recommends = recommends; }

    const CDependencyList & freshens() const { return _freshens; }
    void setFreshens (const CDependencyList & freshens) { _freshens = freshens; }

};

///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

#endif // _ResItem_h
