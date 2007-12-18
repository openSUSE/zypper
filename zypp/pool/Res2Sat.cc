/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/pool/Res2Sat.cc
 *
*/
#include <iostream>
#include <boost/mpl/int.hpp>

#include "zypp/base/Easy.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Exception.h"
#include "zypp/base/String.h"

#include "zypp/ResObject.h"
#include "zypp/Capability.h"
#include "zypp/capability/VersionedCap.h"

#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/sat/Solvable.h"
#include "zypp/sat/Repo.h"

using std::endl;



///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace pool
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////

      inline void store( ::Id & where_r, const::std::string & str_r )
      {
        where_r = sat::IdStr( str_r ).id();
      }

      inline void store( ::Offset & where_r, ::_Solvable * slv_r, const Dependencies & dep_r, Dep which_r )
      {
        enum SatIsRreq {
          ISRREQ_NORMAL      = 0,
          ISRREQ_REQUIRES    = 1,
          ISRREQ_PREREQUIRES = 2
        };
        const CapSet & caps( dep_r[which_r] );
        if ( caps.empty() )
          return;
        const CapSet & prereq( dep_r[Dep::PREREQUIRES] );

        for_( it, caps.begin(), caps.end() )
        {
          SatIsRreq isreq = ISRREQ_NORMAL;
          if ( which_r == Dep::REQUIRES )
          {
            isreq = ( prereq.find( *it ) != prereq.end() ) ? ISRREQ_PREREQUIRES : ISRREQ_REQUIRES;
          }

          std::string name;
          Rel         op;
          Edition     ed;
          using capability::VersionedCap;
          VersionedCap::constPtr vercap = capability::asKind<VersionedCap>(*it);
          if ( vercap )
          {
            name = vercap->name();
            op = vercap->op();
            ed = vercap->edition();
          }
          else
          {
            name = (*it).asString();
          }

          where_r = ::repo_addid_dep( slv_r->repo, where_r,
                                      sat::Capability( name, op, ed, (*it).refers() ).id(),
                                      isreq );
        }
      }

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////


    void res2sat( const ResObject::constPtr & res_r, sat::Solvable & slv_r )
    {
      ::_Solvable * slv( slv_r.get() );
      if ( ! ( res_r && slv ) )
      {
        INT << res_r << " -> " << slv_r << endl;
        ZYPP_THROW( Exception( _("Can't store data in NULL object") ) );
      }

      if ( isKind<Package>( res_r ) )
      {
        store( slv->name, res_r->name() );
      }
      else
      {
        // non-packages prefixed by kind
        store( slv->name, str::form( "%s:%s",
                                     res_r->kind().asString().c_str(),
                                     res_r->name().c_str() ) );
      }
      store( slv->arch,   res_r->arch().asString() );
      store( slv->evr,    res_r->edition().asString() );
      store( slv->vendor, res_r->vendor() );

      store( slv->provides,    slv, res_r->deps(), Dep::PROVIDES );
      store( slv->obsoletes,   slv, res_r->deps(), Dep::OBSOLETES );
      store( slv->conflicts,   slv, res_r->deps(), Dep::CONFLICTS );
      store( slv->requires,    slv, res_r->deps(), Dep::REQUIRES ); // incl. PREREQIRES
      store( slv->recommends,  slv, res_r->deps(), Dep::RECOMMENDS );
      store( slv->suggests,    slv, res_r->deps(), Dep::SUGGESTS );
      store( slv->supplements, slv, res_r->deps(), Dep::SUPPLEMENTS );
      store( slv->enhances,    slv, res_r->deps(), Dep::ENHANCES );
      store( slv->freshens,    slv, res_r->deps(), Dep::FRESHENS );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace pool
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
