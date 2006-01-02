/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Match.h
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

#ifndef _Match_h
#define _Match_h

#include <iosfwd>
#include <string.h>

#include <y2util/Ustring.h>

#include <zypp/solver/detail/MatchPtr.h>
#include <zypp/solver/detail/Channel.h>
#include <zypp/solver/detail/Importance.h>
#include <zypp/solver/detail/ResItem.h>
#include <zypp/solver/detail/WorldPtr.h>
#include <zypp/solver/detail/XmlNode.h>

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

      class Match;
      typedef std::list<constMatchPtr> MatchList;
      
      class World;
      typedef bool (*MatchFn) (constMatchPtr match, void *data);
      
      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : Match
      /**
       *
       **/
      class Match : public CountedRep {
          REP_BODY(Match);
      
        private:
          std::string _channel_id;
      
          Capability _dependency;
      
          std::string _name_glob;
      //    GPatternSpec *_pattern_spec;
      
          Importance _importance;
          bool _importance_gteq;
      
        public:
      
          Match();
          Match(XmlNodePtr node);
          virtual ~Match();
      
          // ---------------------------------- I/O
      
          static std::string toString ( const Match & lock );
      
          virtual std::ostream & dumpOn( std::ostream & str ) const;
      
          friend std::ostream& operator<<( std::ostream&, const Match & lock );
      
          std::string asString ( void ) const;
      
          XmlNodePtr asXmlNode (void) const;
      
          // ---------------------------------- accessors
      
          const std::string & channelId () const { return _channel_id; }
          void setChannel (constChannelPtr channel) { _channel_id = channel->id(); }
          void setChannelId (const std::string & channel_id) { _channel_id = channel_id; }
      
          const Capability & dependency () const { return _dependency; }
          void setDependency (const Capability & dependency) { _dependency = dependency; }
      
          const std::string & glob () const { return _name_glob; }
          void setGlob (const std::string & glob_str) { _name_glob = glob_str; }
      
          const Importance & importance (bool *match_gteq) const { *match_gteq = _importance_gteq; return _importance; }
          void setImportance (const Importance & importance, bool match_gteq) { _importance = importance; _importance_gteq = match_gteq; }
      
          // ---------------------------------- methods
      
          typedef bool (*MatchFn) (constMatchPtr, void *data);
      
          // equality
          bool equals (const Match & match) const;
      
          bool test (constResItemPtr resItem, WorldPtr world) const;
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

#endif // _Match_h
