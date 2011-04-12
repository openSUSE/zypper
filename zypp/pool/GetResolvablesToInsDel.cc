/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/pool/GetResolvablesToInsDel.cc
 *
*/
#include <iostream>
#include <set>

#include "zypp/base/LogTools.h"
#include "zypp/base/LogControl.h"
#include "zypp/sat/Solvable.h"
#include "zypp/sat/WhatObsoletes.h"
#include "zypp/pool/GetResolvablesToInsDel.h"
#include "zypp/pool/PoolStats.h"
#include "zypp/solver/detail/InstallOrder.h"

#include "zypp/Resolver.h"
#include "zypp/sat/Transaction.h"

using std::endl;
using zypp::solver::detail::InstallOrder;

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::GetResolvablesToInsDel"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace pool
  { /////////////////////////////////////////////////////////////////

    /******************************************************************
     **
     **	FUNCTION NAME : strip_obsoleted_to_delete
     **	FUNCTION TYPE : void
     **
     ** strip packages to_delete which get obsoleted by
     ** to_install (i.e. delay deletion in case the
     ** obsoleting package likes to save whatever...
    */

    static void
    strip_obsoleted_to_delete( GetResolvablesToInsDel::PoolItemList & deleteList_r,
                               const GetResolvablesToInsDel::PoolItemList & instlist_r )
    {
      if ( deleteList_r.size() == 0 || instlist_r.size() == 0 )
        return; // ---> nothing to do

      // These are all installed packages obsoleted by any package to be installed.
      // Actually we expect all of them to appear in the deleteList_r, and we want
      // to kick them out. Rpm will delete them when the obsoleter gets installed.
      sat::WhatObsoletes obsoleted( instlist_r.begin(), instlist_r.end() );
      for_( it, obsoleted.poolItemBegin(), obsoleted.poolItemEnd() )
      {
        DBG << "Ignore appl_delete (should be obsoleted): " << *it << endl;
        deleteList_r.remove( *it );
      }

      MIL << "Undelayed deletes: " << deleteList_r << endl;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : GetResolvablesToInsDel::GetResolvablesToInsDel
    //	METHOD TYPE : Ctor
    //
    GetResolvablesToInsDel::GetResolvablesToInsDel( ResPool pool_r, Order order_r )
    {
      zypp::base::LogControl::TmpLineWriter shutUp;
      typedef std::set<PoolItem> PoolItemSet;

      PoolItemList & dellist_r( _toDelete );
      PoolItemList & instlist_r( _toInstall );
      PoolItemList & srclist_r( _toSrcinstall );

      for ( ResPool::const_iterator it = pool_r.begin(); it != pool_r.end(); ++it )
        {
          if (it->status().isToBeInstalled())
            {
              if ((*it)->kind() == ResKind::srcpackage) {
		srclist_r.push_back( *it );
              }
              else
		instlist_r.push_back( *it );
            }
          else if (it->status().isToBeUninstalled())
            {
              if ( it->status().isToBeUninstalledDueToObsolete() )
                {
                  DBG << "Ignore auto_delete (should be obsoleted): " << *it << endl;
                }
              else if ( it->status().isToBeUninstalledDueToUpgrade() )
                {
                  DBG << "Ignore auto_delete (should be upgraded): " << *it << endl;
                }
              else {
		dellist_r.push_back( *it );
              }
            }
        }

      MIL << "ResolvablesToInsDel: delete " << dellist_r.size()
      << ", install " << instlist_r.size()
      << ", srcinstall " << srclist_r.size() << endl;

      ///////////////////////////////////////////////////////////////////
      //
      // strip packages to_delete which get obsoleted by
      // to_install (i.e. delay deletion in case the
      // obsoleting package likes to save whatever...
      //
      ///////////////////////////////////////////////////////////////////
      strip_obsoleted_to_delete( dellist_r, instlist_r );

      if ( dellist_r.size() ) {
        ///////////////////////////////////////////////////////////////////
        //
        // sort delete list...
        //
        ///////////////////////////////////////////////////////////////////
        PoolItemSet delset( dellist_r.begin(), dellist_r.end() );  // for delete order
        PoolItemSet dummy; // dummy, empty, should contain already installed

        InstallOrder order( pool_r, delset, dummy ); // sort according top prereq
        order.init();
        const PoolItemList dsorted( order.getTopSorted() );

        dellist_r.clear();
        for ( PoolItemList::const_reverse_iterator cit = dsorted.rbegin();
              cit != dsorted.rend(); ++cit )
          {
            dellist_r.push_back( *cit );
          }
      }

      ///////////////////////////////////////////////////////////////////
      //
      // sort installed list...
      //
      ///////////////////////////////////////////////////////////////////
      if ( instlist_r.empty() )
        return;

      ///////////////////////////////////////////////////////////////////
      // Compute install order according to packages prereq.
      // Try to group packages with respect to the desired install order
      ///////////////////////////////////////////////////////////////////
      // backup list for debug purpose.
      // You can as well build the set, clear the list and rebuild it in install order.
      PoolItemList instbackup_r;
      instbackup_r.swap( instlist_r );

      PoolItemSet insset( instbackup_r.begin(), instbackup_r.end() ); // for install order
      PoolItemSet installed; // dummy, empty, should contain already installed

      InstallOrder order( pool_r, insset, installed );
      // start recursive depth-first-search
      order.init();
      MIL << "order.init() done" << endl;
      order.printAdj( XXX, false );
      ///////////////////////////////////////////////////////////////////
      // build install list in install order
      ///////////////////////////////////////////////////////////////////
      PoolItemList best_list;
      sat::detail::RepoIdType best_prio     = 0;
      unsigned best_medianum = 0;

      PoolItemList last_list;
      sat::detail::RepoIdType last_prio     = 0;
      unsigned last_medianum = 0;

      PoolItemList other_list;

      for ( PoolItemList items = order.computeNextSet(); ! items.empty(); items = order.computeNextSet() )
        {
          XXX << "order.computeNextSet: " << items.size() << " resolvables" << endl;
          ///////////////////////////////////////////////////////////////////
          // items contains all objects we could install now. Pick all objects
          // from current media, or best media if none for current. Alwayys pick
          // objects that do not require media access.
          ///////////////////////////////////////////////////////////////////

          best_list.clear();
          last_list.clear();
          other_list.clear();

          for ( PoolItemList::iterator cit = items.begin(); cit != items.end(); ++cit )
            {
              ResObject::constPtr cobj( cit->resolvable() );
              if (!cobj)
                continue;

              if ( ! cobj->mediaNr() ) {
                XXX << "No media access required for " << *cit << endl;
                order.setInstalled( *cit );
                other_list.push_back( *cit );
                continue;
              }

              if ( cobj->satSolvable().repository().id() == last_prio &&
                   cobj->mediaNr() == last_medianum ) {
                // prefer packages on current media.
                XXX << "Stay with current media " << *cit << endl;
                last_list.push_back( *cit );
                continue;
              }

              if ( last_list.empty() ) {
                // check for best media as long as there are no packages for current media.

                if ( ! best_list.empty() ) {

                  if ( order_r == ORDER_BY_MEDIANR )
                    {
                      if ( cobj->mediaNr() < best_medianum ) {
                        best_list.clear(); // new best
                      } else if ( cobj->mediaNr() == best_medianum ) {
                        if ( cobj->satSolvable().repository().id() < best_prio ) {
                          best_list.clear(); // new best
                        } else if ( cobj->satSolvable().repository().id() == best_prio ) {
                          XXX << "Add to best list " << *cit << endl;
                          best_list.push_back( *cit ); // same as best -> add
                          continue;
                        } else {
                          continue; // worse
                        }
                      } else {
                        continue; // worse
                      }
                    }
                  else // default: ORDER_BY_SOURCE
                    {
                      if ( cobj->satSolvable().repository().id() < best_prio ) {
                        best_list.clear(); // new best
                      } else if ( cobj->satSolvable().repository().id() == best_prio ) {
                        if ( cobj->mediaNr() < best_medianum ) {
                          best_list.clear(); // new best
                        } else if ( cobj->mediaNr() == best_medianum ) {
                          XXX << "Add to best list " << *cit << endl;
                          best_list.push_back( *cit ); // same as best -> add
                          continue;
                        } else {
                          continue; // worse
                        }
                      } else {
                        continue; // worse
                      }
                    }
                }

                if ( best_list.empty() )
                  {
                    XXX << "NEW BEST LIST [S" << cobj->satSolvable().repository().id() << ":" << cobj->mediaNr()
                        << "] (last [S" << best_prio << ":" << best_medianum << "])" << endl;
                    best_prio     = cobj->satSolvable().repository().id();
                    best_medianum = cobj->mediaNr();
                    // first package or new best
                    XXX << "Add to best list " << *cit << endl;
                    best_list.push_back( *cit );
                    continue;
                  }
              }

            } // for all objects in current set

          ///////////////////////////////////////////////////////////////////
          // remove objects picked from install order and append them to
          // install list.
          ///////////////////////////////////////////////////////////////////
          PoolItemList & take_list( last_list.empty() ? best_list : last_list );
          if ( last_list.empty() )
            {
              MIL << "SET NEW media [S" << best_prio << ":" << best_medianum << "]" << endl;
              last_prio     = best_prio;
              last_medianum = best_medianum;
            }
          else
            {
              XXX << "SET CONTINUE [S" << best_prio << ":" << best_medianum << "]" << endl;
            }

          for ( PoolItemList::iterator it = take_list.begin(); it != take_list.end(); ++it )
            {
              order.setInstalled( *it );
              XXX << "SET collect " << (*it) << endl;
            }
          // move everthing from take_list to the end of instlist_r, clean take_list
          instlist_r.splice( instlist_r.end(), take_list );
          // same for other_list
          instlist_r.splice( instlist_r.end(), other_list );

        } // for all sets computed


      MIL << "order done" << endl;
      if ( instbackup_r.size() != instlist_r.size() )
        {
          ERR << "***************** Lost packages in InstallOrder sort." << endl;
        }

    }

    void GetResolvablesToInsDel::debugDiffTransaction() const
    {
      SEC << "START debugDiffTransaction" << endl;
      sat::Transaction trans( ResPool::instance().resolver().getTransaction() );

      {
	const PoolItemList & clist( _toDelete );
	for_( it, clist.begin(), clist.end() )
	{
	  sat::Transaction::const_iterator ci( trans.find( *it ) );
	  if ( ci == trans.end() )
	    ERR << "Missing to del in NEW trans: " << *it << endl;
	}
      }
      {
	const PoolItemList & clist( _toInstall );
	for_( it, clist.begin(), clist.end() )
	{
	  sat::Transaction::const_iterator ci( trans.find( *it ) );
	  if ( ci == trans.end() )
	    ERR << "Missing to ins in NEW trans: " << *it << endl;
	}
      }
      {
	const PoolItemList & clist( _toSrcinstall );
	for_( it, clist.begin(), clist.end() )
	{
	  sat::Transaction::const_iterator ci( trans.find( *it ) );
	  if ( ci == trans.end() )
	    ERR << "Missing srcins in NEW trans: " << *it << endl;
	}
      }

      SEC << "END debugDiffTransaction" << endl;
    }

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const GetResolvablesToInsDel & obj )
    {
      dumpPoolStats( str << "toInstall: " << endl,
                     obj._toInstall.begin(), obj._toInstall.end() ) << endl;
      dumpPoolStats( str << "toDelete: " << endl,
                     obj._toDelete.begin(), obj._toDelete.end() ) << endl;
      return str;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace pool
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

