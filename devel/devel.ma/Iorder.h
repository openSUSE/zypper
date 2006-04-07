
/******************************************************************
 **
 **
 **	FUNCTION NAME : strip_obsoleted_to_delete
 **	FUNCTION TYPE : void
 **
 ** strip packages to_delete which get obsoleted by
 ** to_install (i.e. delay deletion in case the
 ** obsoleting package likes to save whatever...
*/

static void
strip_obsoleted_to_delete( PoolItemList & deleteList_r,
                           const PoolItemList & instlist_r )
{
  if ( deleteList_r.size() == 0 || instlist_r.size() == 0 )
    return; // ---> nothing to do

  // build obsoletes from instlist_r
  CapSet obsoletes;
  for ( PoolItemList::const_iterator it = instlist_r.begin();
	it != instlist_r.end(); ++it )
    {
      PoolItem_Ref item( *it );
      obsoletes.insert( item->dep(Dep::OBSOLETES).begin(), item->dep(Dep::OBSOLETES).end() );
    }
  if ( obsoletes.size() == 0 )
    return; // ---> nothing to do

  // match them... ;(
  PoolItemList undelayed;
  // forall applDelete Packages...
  for ( PoolItemList::iterator it = deleteList_r.begin();
	it != deleteList_r.end(); ++it )
    {
      PoolItem_Ref ipkg( *it );
      bool delayPkg = false;
      // ...check whether an obsoletes....
      for ( CapSet::iterator obs = obsoletes.begin();
            ! delayPkg && obs != obsoletes.end(); ++obs )
        {
          // ...matches anything provided by the package?
          for ( CapSet::const_iterator prov = ipkg->dep(Dep::PROVIDES).begin();
                prov != ipkg->dep(Dep::PROVIDES).end(); ++prov )
            {
              if ( obs->matches( *prov ) == CapMatch::yes )
                {
                  // if so, delay package deletion
                  DBG << "Ignore appl_delete (should be obsoleted): " << ipkg << endl;
                  delayPkg = true;
                  ipkg.status().setTransact( false, ResStatus::USER );
                  break;
                }
            }
        }
      if ( ! delayPkg ) {
        DBG << "undelayed " << ipkg << endl;
        undelayed.push_back( ipkg );
      }
    }
  // Puhh...
  deleteList_r.swap( undelayed );
}




void
getResolvablesToInsDel ( const ResPool pool_r,
                         PoolItemList & dellist_r,
                         PoolItemList & instlist_r,
                         PoolItemList & srclist_r )
{
  dellist_r.clear();
  instlist_r.clear();
  srclist_r.clear();
  PoolItemList nonpkglist;

  for ( ResPool::const_iterator it = pool_r.begin(); it != pool_r.end(); ++it )
    {
      if (it->status().isToBeInstalled())
	{
          if ((*it)->kind() == ResTraits<SrcPackage>::kind) {
            srclist_r.push_back( *it );
          }
          else if ((*it)->kind() != ResTraits<Package>::kind) {
            nonpkglist.push_back( *it );
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
  << ", srcinstall " << srclist_r.size()
  << ", nonpkg " << nonpkglist.size() << endl;

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
  if ( instlist_r.empty() ) {
    instlist_r.splice( instlist_r.end(), nonpkglist );

    return;
  }
#warning Source Rank Priority ?
#if 0
  ///////////////////////////////////////////////////////////////////
  // Get desired order of InstSrc'es to install from.
  ///////////////////////////////////////////////////////////////////
  typedef map<unsigned,unsigned> RankPriority;

  RankPriority rankPriority;
  {
    InstSrcManager::ISrcIdList sourcerank( Y2PM::instSrcManager().instOrderSources() );
    // map InstSrc rank to install priority
    unsigned prio = 0;
    for ( InstSrcManager::ISrcIdList::const_iterator it = sourcerank.begin();
          it != sourcerank.end(); ++it, ++prio ) {
      rankPriority[(*it)->descr()->default_rank()] = prio;
    }
  }
#endif

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
  // NOTE: for now _prio is Source numeric Id.
  ///////////////////////////////////////////////////////////////////
  PoolItemList best_list;
  unsigned best_prio     = 0;
  unsigned best_medianum = 0;

  PoolItemList last_list;
  unsigned last_prio     = 0;
  unsigned last_medianum = 0;

  PoolItemList other_list;

  for ( PoolItemList items = order.computeNextSet(); ! items.empty(); items = order.computeNextSet() )
    {
      MIL << "order.computeNextSet: " << items.size() << " resolvables" << endl;
      ///////////////////////////////////////////////////////////////////
      // items contains all packages we could install now. Pick all packages
      // from current media, or best media if none for current.
      ///////////////////////////////////////////////////////////////////

      best_list.clear();
      last_list.clear();
      other_list.clear();

      for ( PoolItemList::iterator cit = items.begin(); cit != items.end(); ++cit )
        {
          Resolvable::constPtr res( cit->resolvable() );
          if (!res) continue;
          Package::constPtr cpkg( asKind<Package>(res) );
          if (!cpkg) {
	    XXX << "Not a package " << *cit << endl;
	    order.setInstalled( *cit );
	    other_list.push_back( *cit );
	    continue;
          }
          XXX << "Package " << *cpkg << ", media " << cpkg->mediaId() << " last_medianum " << last_medianum << " best_medianum " << best_medianum << endl;
          if ( cpkg->source().numericId() == last_prio &&
               cpkg->mediaId() == last_medianum ) {
            // prefer packages on current media.
            last_list.push_back( *cit );
            continue;
          }

          if ( last_list.empty() ) {
            // check for best media as long as there are no packages for current media.

            if ( ! best_list.empty() ) {

              if ( cpkg->source().numericId() < best_prio ) {
                best_list.clear(); // new best
              } else if ( cpkg->source().numericId() == best_prio ) {
                if ( cpkg->mediaId() < best_medianum ) {
                  best_list.clear(); // new best
                } else if ( cpkg->mediaId() == best_medianum ) {
                  best_list.push_back( *cit ); // same as best -> add
                  continue;
                } else {
                  continue; // worse
                }
              } else {
                continue; // worse
              }
            }

            if ( best_list.empty() )
              {
                // first package or new best
                best_list.push_back( *cit );
                best_prio     = cpkg->source().numericId();
                best_medianum = cpkg->mediaId();
                continue;
              }
          }

        } // for all packages in current set

      ///////////////////////////////////////////////////////////////////
      // remove packages picked from install order and append them to
      // install list.
      ///////////////////////////////////////////////////////////////////
      PoolItemList & take_list( last_list.empty() ? best_list : last_list );
      if ( last_list.empty() )
        {
          MIL << "SET NEW media " << best_medianum << endl;
          last_prio     = best_prio;
          last_medianum = best_medianum;
        }
      else
        {
          MIL << "SET CONTINUE" << endl;
        }

      for ( PoolItemList::iterator it = take_list.begin(); it != take_list.end(); ++it )
        {
          order.setInstalled( *it );
          DBG << "SET isrc " << (*it)->source().numericId() << " -> " << (*it) << endl;
        }
      // move everthing from take_list to the end of instlist_r, clean take_list
      instlist_r.splice( instlist_r.end(), take_list );
      // same for other_list
      instlist_r.splice( instlist_r.end(), other_list );

    } // for all sets computed


  if ( instbackup_r.size() != instlist_r.size() )
    {
      ERR << "***************** Lost packages in InstallOrder sort." << endl;
    }
  instlist_r.splice( instlist_r.end(), nonpkglist );
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
 inline  unsigned mediaId( const PoolItem & pi )
  {
    Package::constPtr pkg( asKind<Package>(pi.resolvable()) );
    if ( pkg )
      return pkg->mediaId();
    return 0;
  }
PoolItemList commit( const PoolItemList & items_r, bool cont )
{
  static unsigned  go = 0;
  static unsigned  hop = 0;
  static unsigned  itm = 0;
  static unsigned  sid = 0;
  static unsigned  mid = 0;

  MIL << "=====[" << ++go << "]========================================" << endl;
  if ( ! cont )
    {
      MIL << "DONE  on item " << itm << endl;
      return PoolItemList();
    }

  for ( PoolItemList::const_iterator it = items_r.begin(); it != items_r.end(); ++it )
    {
      ++itm;
      unsigned cmid = mediaId( *it );
      unsigned csid = (*it)->source().numericId();

      if ( cmid && ( cmid != mid || csid != sid ) )
        {
          MIL << "Hop " << ++hop << ": "
              << "S" << sid << "/M" << mid
              << " -> ";
          sid = csid;
          mid = cmid;
          MIL << "S" << sid << "/M" << mid
              << " on item " << itm
              << endl;
        }
    }

  return PoolItemList();
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
int commit( ResPool pool_r, unsigned int medianr,
            PoolItemList & errors_r,
            PoolItemList & remaining_r,
            PoolItemList & srcremaining_r, bool dry_run )
{
  MIL << "TargetImpl::commit(<pool>, " << medianr << ")" << endl;
  dry_run = true;

  errors_r.clear();
  remaining_r.clear();
  srcremaining_r.clear();

  PoolItemList to_uninstall;
  PoolItemList to_install;
  PoolItemList to_srcinstall;
  getResolvablesToInsDel( pool_r, to_uninstall, to_install, to_srcinstall );

  if ( medianr ) {
    MIL << "Restrict to media number " << medianr << endl;
  }

  //commit (to_uninstall, dry_run );

  if (medianr == 0) {			// commit all
    remaining_r = commit( to_install, dry_run );
    srcremaining_r = commit( to_srcinstall, dry_run );
  }
  else
    {
      PoolItemList current_install;
      PoolItemList current_srcinstall;

      for (PoolItemList::iterator it = to_install.begin(); it != to_install.end(); ++it)
        {
          Resolvable::constPtr res( it->resolvable() );
          Package::constPtr pkg( asKind<Package>(res) );
          if (pkg && medianr != pkg->mediaId())								// check medianr for packages only
            {
              XXX << "Package " << *pkg << ", wrong media " << pkg->mediaId() << endl;
              remaining_r.push_back( *it );
            }
          else
            {
              current_install.push_back( *it );
            }
        }
      PoolItemList bad = commit( current_install, dry_run );
      remaining_r.insert(remaining_r.end(), bad.begin(), bad.end());

      for (PoolItemList::iterator it = to_srcinstall.begin(); it != to_srcinstall.end(); ++it)
        {
          Resolvable::constPtr res( it->resolvable() );
          Package::constPtr pkg( asKind<Package>(res) );
          if (pkg && medianr != pkg->mediaId()) // check medianr for packages only
            {
              XXX << "Package " << *pkg << ", wrong media " << pkg->mediaId() << endl;
              srcremaining_r.push_back( *it );
            }
          else {
            current_srcinstall.push_back( *it );
          }
        }
      bad = commit( current_srcinstall, dry_run );
      srcremaining_r.insert(srcremaining_r.end(), bad.begin(), bad.end());
    }
  commit( PoolItemList(), false );
  return to_install.size() - remaining_r.size();
}
