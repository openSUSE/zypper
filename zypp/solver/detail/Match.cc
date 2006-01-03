/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Match.cc
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

#include <y2util/stringutil.h>

#include <zypp/solver/detail/Match.h>
#include <zypp/solver/detail/World.h>
#include <zypp/Rel.h>

#include <zypp/CapFactory.h>
#include <zypp/CapSet.h>


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
      
      IMPL_BASE_POINTER(Match);
      
      //---------------------------------------------------------------------------
      
      string
      Match::asString ( void ) const
      {
          return toString (*this);
      }
      
      
      string
      Match::toString ( const Match & lock )
      {
          return "<lock/>";
      }
      
      
      ostream &
      Match::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream& os, const Match& edition)
      {
          return os << edition.asString();
      }
      
      //---------------------------------------------------------------------------
      
      Match::Match()
          : _importance (Importance::Undefined)
      {
      }
      
      Match::Match (XmlNodePtr node)
          : _importance (Importance::Undefined)
      {
      }
      
      Match::~Match()
      {
      }
      
      //---------------------------------------------------------------------------
      
      XmlNodePtr
      Match::asXmlNode (void) const
      {
          return new XmlNode("match");
      }
      
      
      // equality
      bool
      Match::equals ( const Match & lock ) const {
      
          // Check the name glob
      
          if ((_name_glob.empty()) ^ (lock._name_glob.empty()))
      	return false;
          if (_name_glob != lock._name_glob)
      	return false;
      
          // Check the channel
      
          if ((_channel_id.empty()) ^ (lock._channel_id.empty()))
      	return false;
          if (_channel_id != lock._channel_id)
      	return false;
      
          // Check the importance
      
          if (_importance != lock._importance
      	|| _importance_gteq != lock._importance_gteq)
          {
      	return false;
          }
      
          // Check the dep
          if ( _dependency != lock._dependency)
              return false;
      
          return true;
      }
      
      
      bool
      Match::test (constResItemPtr resItem, WorldPtr world) const
      {
          string name;
          constChannelPtr channel = resItem->channel ();
        
          if (channel != NULL && !_channel_id.empty()) {
              if (! channel->hasEqualId (_channel_id)) {
                  return false;
              }
          }
      
          name = resItem->name ();
      
          // FIXME, implement regexp
#if 0
          if (match->_pattern_spec
              && ! g_pattern_match_string (match->pattern_spec, name)) {
              return false;
          }
#endif
      
          /* FIXME: ResItems don't have ResItemUpdate right now */
          /*   if (match->importance != RC_IMPORTANCE_INVALID && */
          /* 	  !rc_resItem_is_installed (resItem)) { */
          /* 	  RCResItemUpdate *up = rc_resItem_get_latest_update (pkg); */
          /* 	  if (up) { */
          /* 		  if (match->importance_gteq ? up->importance > match->importance */
          /* 			  : up->importance < match->importance) */
          /* 			  return FALSE; */
          /* 	  } */
          /*   } */
          CapFactory  factory;                      
          Capability dependency;
          bool check = false;
      
          dependency = factory.parse ( resItem->kind(),
                                       resItem->name(),
                                       Rel::EQ,
                                       resItem->edition());
//          check = _dependency.matches (dependency);
          return check;
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

