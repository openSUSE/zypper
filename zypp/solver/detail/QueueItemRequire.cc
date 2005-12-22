/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* QueueItemRequire.cc
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

#include <zypp/solver/detail/QueueItemRequire.h>
#include <zypp/solver/detail/QueueItemBranch.h>
#include <zypp/solver/detail/QueueItemUninstall.h>
#include <zypp/solver/detail/QueueItemInstall.h>
#include <zypp/solver/detail/QueueItem.h>
#include <zypp/solver/detail/ResolverContext.h>
#include <zypp/solver/detail/ResolverInfoDependsOn.h>
#include <zypp/solver/detail/ResolverInfoMisc.h>
#include <zypp/solver/detail/ResolverInfoNeededBy.h>
#include <zypp/solver/detail/World.h>

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
      
      IMPL_DERIVED_POINTER(QueueItemRequire,QueueItem);
      
      //---------------------------------------------------------------------------
      
      string
      QueueItemRequire::asString ( void ) const
      {
          return toString (*this);
      }
      
      
      string
      QueueItemRequire::toString ( const QueueItemRequire & item)
      {
          string ret = "[Require: ";
          ret += item._dep->asString();
          if (item._requiring_resItem != NULL) {
      	ret += ", Required by ";
      	ret += item._requiring_resItem->asString();
          }
          if (item._upgraded_resItem != NULL) {
      	ret += ", Upgrades ";
      	ret += item._upgraded_resItem->asString();
          }
          if (item._lost_resItem != NULL) {
      	ret += ", Lost ";
      	ret += item._lost_resItem->asString();
          }
          if (item._remove_only) ret += ", Remove Only";
          if (item._is_child) ret += ", Child";
          ret += "]";
      
          return ret;
      }
      
      
      ostream &
      QueueItemRequire::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream& os, const QueueItemRequire & item)
      {
          return os << item.asString();
      }
      
      //---------------------------------------------------------------------------
      
      QueueItemRequire::QueueItemRequire (WorldPtr world, constDependencyPtr dep)
          : QueueItem (QUEUE_ITEM_TYPE_REQUIRE, world)
          , _dep (dep)
          , _requiring_resItem (NULL)
          , _upgraded_resItem (NULL)
          , _lost_resItem (NULL)
          , _remove_only (false)
          , _is_child (false)
      {
      }
      
      
      QueueItemRequire::~QueueItemRequire()
      {
      }
      
      //---------------------------------------------------------------------------
      
      void
      QueueItemRequire::addResItem (constResItemPtr resItem)
      {
          assert (_requiring_resItem == NULL);
          _requiring_resItem = resItem;
      }
      
      
      //---------------------------------------------------------------------------
      
      typedef std::map <constSpecPtr, bool> UniqTable;
      
      typedef struct {
          constResItemPtr resItem;
          constSpecPtr dep;
          ResolverContextPtr context;
          WorldPtr world;
          CResItemList providers;
          UniqTable *uniq;
      } RequireProcessInfo;
      
      
      static bool
      require_process_cb (constResItemPtr resItem, constSpecPtr spec, void *data)
      {
          RequireProcessInfo *info = (RequireProcessInfo *)data;
          ResItemStatus status;
      
          status = info->context->getStatus (resItem);
      //fprintf (stderr, "require_process_cb(res: %s, spec %s, status %s)\n", resItem->asString().c_str(), spec->asString().c_str(), ResolverContext::toString(status).c_str());
      //fprintf (stderr, "require_process_cb(info->dep: %s)\n", info->dep ? info->dep->asString().c_str() : "(null)");
      //fprintf (stderr, "require_process_cb(resItemIsPossible -> %d)\n", info->context->resItemIsPossible (resItem));
          /* info->dep is set for resItem set childern only. If it is set
             allow only exactly required version */
          if (info->dep != NULL
      	&& !info->dep->equals(spec)) {
      	return true;
          }
      
          if ((! resItem_status_is_to_be_uninstalled (status))
      	&& ! info->context->isParallelInstall (resItem)
      	&& info->uniq->find((constSpecPtr)resItem) == info->uniq->end()
      	&& info->context->resItemIsPossible (resItem)
      	&& ! info->world->resItemIsLocked (resItem)) {
      
      	info->providers.push_front (resItem);
      	(*(info->uniq))[resItem] = true;
          }
      
          return true;
      }
      
      
      static bool
      no_installable_providers_info_cb (constResItemPtr resItem, constSpecPtr spec, void *data)
      {
          RequireProcessInfo *info = (RequireProcessInfo *)data;
          ResItemStatus status;
          string msg_str;
      
          status = info->context->getStatus (resItem);
      
          if (resItem_status_is_to_be_uninstalled (status)) {
      	msg_str = resItem->name() + " provides " + spec->asString() + ", but is scheduled to be uninstalled.";
          } else if (info->context->isParallelInstall (resItem)) {
      	msg_str = resItem->name() + " provides " + spec->asString() + ", but another version of that resItem is already installed.";
          } else if (! info->context->resItemIsPossible (resItem)) {
      	msg_str = resItem->name() + " provides " + spec->asString() + ", but it is uninstallable.  Try installing it on its own for more details.";
          } else if (info->world->resItemIsLocked (resItem)) {
      	msg_str = resItem->name() + " provides " + spec->asString() + ", but it is locked.";
          }
      
          if (!msg_str.empty()) {
      	info->context->addInfoString (info->resItem, RESOLVER_INFO_PRIORITY_VERBOSE, msg_str);
          }
          
          return true;
      }
      
      
      static bool
      look_for_upgrades_cb (constResItemPtr resItem, void *data)
      {
          CResItemList *rl = (CResItemList *)data;
          rl->push_front (resItem);
          return true;
      }
      
      
      static bool
      codependent_resItems (constResItemPtr r1, constResItemPtr r2)
      {
          string name1 = r1->name();
          string name2 = r2->name();
          int len1 = name1.size();
          int len2 = name2.size();
      
          if (len2 < len1) {
      	string swap = name1;
      	int swap_len = len1;
      	name1 = name2;
      	name2 = swap;
      	len1 = len2;
      	len2 = swap_len;
          }
      
          // foo and foo-bar are automatically co-dependent
          if (len1 < len2
      	&& strncmp (name1.c_str(), name2.c_str(), len1) == 0
      	&& name2[len1] == '-') {
      	return true;
          }
          
          return false;
      }
      
      
      bool
      QueueItemRequire::process (ResolverContextPtr context, QueueItemList & new_items)
      {
          if (getenv ("RC_SPEW")) fprintf (stderr, "QueueItemRequire::process(%s)\n", this->asString().c_str());
      
          if (context->requirementIsMet (_dep, _is_child)) {
      	if (getenv ("RC_SPEW")) fprintf (stderr, "requirement is already met in current context\n");
      //    rc_queue_item_free (item);
      	return true;
          }
      
          RequireProcessInfo info;
      
          info.resItem = _requiring_resItem;
          info.dep = _is_child ? _dep : NULL;
          info.context = context;
          info.world = world();
          info.uniq = new UniqTable();		//FIXME: op: g_hash_table_new (rc_resItem_spec_hash, rc_resItem_spec_equal);
      
          int num_providers = 0;
      
          if (! _remove_only) {
      
      	world()->foreachProvidingResItem (_dep, require_process_cb, &info);
      	
      	num_providers = info.providers.size();
      
      	if (getenv ("RC_SPEW")) fprintf (stderr, "requirement is met by %d resolvable\n", num_providers);
          }
      
          std::string msg;
      
          if (num_providers == 0) {
      
      	if (getenv ("RC_SPEW")) fprintf (stderr, "Unfulfilled requirement, try different solution\n");
      
      	QueueItemUninstallPtr uninstall_item = NULL;
      	QueueItemBranchPtr branch_item = NULL;
      	bool explore_uninstall_branch = true;
      
      	if (_upgraded_resItem == NULL) {
      	    ResolverInfoPtr err_info;
      
      	    msg = string ("There are no ") + (_remove_only ? "alternative installed" : "installable") + " providers of " + _dep->asString();
      	    if (_requiring_resItem != NULL) {
      		msg += " for ";
      		msg += _requiring_resItem->asString();
      	    }
      
      	    err_info = new ResolverInfoMisc (_requiring_resItem, RESOLVER_INFO_PRIORITY_VERBOSE, msg);
      
      	    context->addInfo (err_info);
      
      	    // Maybe we can add some extra info on why none of the providers are suitable.
      	    world()->foreachProvidingResItem (_dep, no_installable_providers_info_cb, (void *)&info);
      	}
      	
      	// If this is an upgrade, we might be able to avoid removing stuff by upgrading it instead.
      	if (_upgraded_resItem != NULL
      	    && _requiring_resItem != NULL) {
      
      	    CResItemList upgrade_list;
      
      	    world()->foreachUpgrade (_requiring_resItem, new Channel(CHANNEL_TYPE_ANY), look_for_upgrades_cb, (void *)&upgrade_list);
      
      	    if (!upgrade_list.empty()) {
      		string label, req_str, up_str;
      
      		branch_item = new QueueItemBranch (world());
      
      		req_str = _requiring_resItem->asString();
      		up_str  = _upgraded_resItem->asString();
      
      		label = string ("for requiring ") + _dep->asString() + " for " + req_str + " when upgrading " + up_str;
      		branch_item->setLabel (label);
      //fprintf (stderr, "Branching: %s\n", label.c_str());
      		for (CResItemList::const_iterator iter = upgrade_list.begin(); iter != upgrade_list.end(); iter++) {
      		    constResItemPtr upgrade_resItem = *iter;
      		    QueueItemInstallPtr install_item;
      
      		    if (context->resItemIsPossible (upgrade_resItem)) {
      		    
      			install_item = new QueueItemInstall (world(), upgrade_resItem);
      		    	install_item->setUpgrades (_requiring_resItem);
      			branch_item->addItem (install_item);
      
      			ResolverInfoNeededByPtr upgrade_info = new ResolverInfoNeededBy (upgrade_resItem);
      			upgrade_info->addRelatedResItem (_upgraded_resItem);
      			install_item->addInfo (upgrade_info);
      
      			// If an upgrade resItem has its requirements met, don't do the uninstall branch.
      			//   FIXME: should we also look at conflicts here?
      
      			if (explore_uninstall_branch) {
      			    CDependencyList requires = upgrade_resItem->requires();
      			    CDependencyList::const_iterator iter = requires.begin();
      			    for (; iter != requires.end(); iter++) {
      				constDependencyPtr req = *iter;
      				if (! context->requirementIsMet (req, false)) {
      					break;
      				}
      			    }
      			    if (iter == requires.end()) {
      				explore_uninstall_branch = false;
      			    }
      			}
      			
      		    } /* if (context->resItemIsPossible ( ... */
      		} /* for (iter = upgrade_list; ... */
      	    } /* if (upgrade_list) ... */
      
      	    if (!upgrade_list.empty()
      		&& branch_item->isEmpty ()) {
      
      		for (CResItemList::const_iterator iter = upgrade_list.begin(); iter != upgrade_list.end(); iter++) {
      		    string str;
      		    string p1, p2;
      
      		    p1 = _requiring_resItem->asString();
      		    p2 = (*iter)->asString();
      		    str = string ("Upgrade to ") + p2 + " to avoid removing " + p1 + " is not possible.";
      
      		    ResolverInfoMiscPtr misc_info = new ResolverInfoMisc (NULL, RESOLVER_INFO_PRIORITY_VERBOSE, str);
      		    misc_info->addRelatedResItem (_requiring_resItem);
      		    misc_info->addRelatedResItem (*iter);
      		    context->addInfo (misc_info);
      
      		    explore_uninstall_branch = true;
      		}
      
      		//
      		//  The exception: we always want to consider uninstalling
      		//  when the requirement has resulted from a resItem losing
      		//  one of it's provides.
      		
      	    } else if (!upgrade_list.empty()
      		       && explore_uninstall_branch
      		       && codependent_resItems (_requiring_resItem, _upgraded_resItem)
      		       && _lost_resItem == NULL) {
      		explore_uninstall_branch = false;
      	    }
      
      	} /* if (_upgrade_resItem && _requiring_resItem) ... */
      
      	// We always consider uninstalling when in verification mode.
      
      	if (context->verifying()) {
      	    explore_uninstall_branch = true;
      	}
      
      	if (explore_uninstall_branch && _requiring_resItem) {
      	    ResolverInfoPtr log_info;
      	    uninstall_item = new QueueItemUninstall (world(),_requiring_resItem, "unsatisfied requirements");
      	    uninstall_item->setDependency (_dep);
      	    
      	    if (_lost_resItem) {
      		log_info = new ResolverInfoDependsOn (_requiring_resItem, _lost_resItem);
      		uninstall_item->addInfo (log_info);
      	    }
      	    
      	    if (_remove_only)
      		uninstall_item->setRemoveOnly ();
      	}
      
      	if (uninstall_item && branch_item) {
      	    branch_item->addItem (uninstall_item);            
      	    new_items.push_front (branch_item);
      	} else if (uninstall_item) {
      	    new_items.push_front (uninstall_item);
      	} else if (branch_item) {
      	    new_items.push_front (branch_item);
      	} else {
      	    // We can't do anything to resolve the missing requirement, so we fail.
      	    string msg = string ("Can't satisfy requirement '") + _dep->asString() + "'";
      	    
      	    context->addErrorString (NULL, msg);
      	}
      	
          } else if (num_providers == 1) {
      
      	if (getenv ("RC_SPEW")) fprintf (stderr, "Found exactly one resolvable, installing it.\n");
      
      	QueueItemInstallPtr install_item = new QueueItemInstall (world(), info.providers.front());
      	install_item->addDependency (_dep);
      
      	// The requiring resItem could be NULL if the requirement was added as an extra dependency.
      	if (_requiring_resItem) {
      	    install_item->addNeededBy (_requiring_resItem);
      	}
      	new_items.push_front (install_item);
      
          } else if (num_providers > 1) {
      
      	if (getenv ("RC_SPEW")) fprintf (stderr, "Found more than one resolvable, branching.\n");
      
      //fprintf (stderr, "Found more than one resItem, branching.\n");
      	QueueItemBranchPtr branch_item = new QueueItemBranch (world());
      
      	for (CResItemList::const_iterator iter = info.providers.begin(); iter != info.providers.end(); iter++) {
      	    QueueItemInstallPtr install_item = new QueueItemInstall (world(), *iter);
      	    install_item->addDependency (_dep);
      	    branch_item->addItem (install_item);
      
      	    // The requiring resItem could be NULL if the requirement was added as an extra dependency.
      	    if (_requiring_resItem) {
      		install_item->addNeededBy (_requiring_resItem);
      	    }
      	}
      
      	new_items.push_front (branch_item);
      
          } else {
      	abort ();
          }
      
         
      //    rc_queue_item_free (item);
          return true;
      }
      
      //---------------------------------------------------------------------------
      
      QueueItemPtr
      QueueItemRequire::copy (void) const
      {
          QueueItemRequirePtr new_require = new QueueItemRequire (world(), _dep);
          ((QueueItemPtr)new_require)->copy((constQueueItemPtr)this);
      
          new_require->_requiring_resItem = _requiring_resItem;
          new_require->_upgraded_resItem  = _upgraded_resItem;
          new_require->_remove_only          = _remove_only;
      
          return new_require;
      }
      
      
      int
      QueueItemRequire::cmp (constQueueItemPtr item) const
      {
          int cmp = this->compare (item);		// assures equal type
          if (cmp != 0)
      	return cmp;
      
          constQueueItemRequirePtr require = item;
      
          cmp = Spec::compare ((constSpecPtr) _dep, ((constSpecPtr)(require->dependency())));
          if (cmp)
      	return cmp;
      
          return _dep->relation() == require->dependency()->relation();
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

