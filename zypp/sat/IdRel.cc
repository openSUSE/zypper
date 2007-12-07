/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/IdRel.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/base/String.h"
#include "zypp/base/Regex.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Exception.h"
#include "zypp/Rel.h"
#include "zypp/Edition.h"

#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/sat/IdRel.h"
#include "zypp/sat/Pool.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////
      detail::IdType relFromStr( ::_Pool * pool_r, const std::string & name_r, Rel op_r, const Edition & ed_r, const KindId & kind_r )
      {
        detail::IdType nid( detail::noId );
#warning Add or not kind package
        if ( ! kind_r )
        {
          nid = IdStr( name_r ).id();
        }
        else
        {
          // non-packages prefixed by kind
          nid = IdStr( str::form( "%s:%s",
                                  kind_r.c_str(),
                                  name_r.c_str() ) ).id();
        }

        if ( op_r != Rel::ANY && ed_r != Edition::noedition )
        {
#warning glue edition
          nid = ::rel2id( pool_r, nid, IdStr( ed_r.asString() ).id(), op_r.bits(), /*create*/true );
        }

        return nid;
      }

      detail::IdType relFromStr( ::_Pool * pool_r, const std::string & str_r, const KindId & kind_r )
      {
        // strval_r has at least two words which could make 'op edition'?
        // improve regex!
        static const str::regex  rx( "(.*[^ \t])([ \t]+)([^ \t]+)([ \t]+)([^ \t]+)" );
        static str::smatch what;

        std::string name( str_r );
        Rel         op;
        Edition     ed;
        if( str_r.find(' ') != std::string::npos
            && str::regex_match( str_r, what, rx ) )
        {
          try
          {
            Rel     cop( what[3] );
            Edition ced( what[5] );
            name = what[1];
            op = cop;
            ed = ced;
          }
          catch ( Exception & excpt )
          {
            // So they don't make valid 'op edition'
            ZYPP_CAUGHT( excpt );
            DBG << "Trying named relation for: " << str_r << endl;
          }
        }
        //else
        // not a versioned relation

        return relFromStr( pool_r, name, op, ed, kind_r );
      }

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    const IdRel IdRel::Null( STRID_NULL );

    /////////////////////////////////////////////////////////////////

    IdRel::IdRel( const char * str_r, const KindId & kind_r  )
      : _id( relFromStr( myPool().getPool(), str_r, kind_r ) )
    {}

    IdRel::IdRel( const std::string & str_r, const KindId & kind_r  )
      : _id( relFromStr( myPool().getPool(), str_r.c_str(), kind_r ) )
    {}

    IdRel::IdRel( const std::string & name_r, Rel op_r, const Edition & ed_r, const KindId & kind_r )
      : _id( relFromStr( myPool().getPool(), name_r, op_r, ed_r, kind_r ) )
    {}

    const char * IdRel::c_str() const
    {
      return ::dep2str( myPool().getPool(), _id );
    }

    std::string IdRel::string() const
    {
      return ::dep2str( myPool().getPool(), _id );
    }

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const IdRel & obj )
    {
      return str << obj.c_str();
    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
