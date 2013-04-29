/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ui/SelectableImpl.h
 *
*/
#ifndef ZYPP_UI_SELECTABLEIMPL_H
#define ZYPP_UI_SELECTABLEIMPL_H

#include <iostream>
#include "zypp/base/LogTools.h"

#include "zypp/base/PtrTypes.h"

#include "zypp/ResPool.h"
#include "zypp/Resolver.h"
#include "zypp/ui/Selectable.h"
#include "zypp/ui/SelectableTraits.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Selectable::Impl
    //
    /** Selectable implementation.
     * \note Implementation is based in PoolItem, just the Selectable
     * inteface restricts them to ResObject::constPtr.
    */
    struct Selectable::Impl
    {
    public:

      typedef SelectableTraits::AvailableItemSet         AvailableItemSet;
      typedef SelectableTraits::available_iterator       available_iterator;
      typedef SelectableTraits::available_const_iterator available_const_iterator;
      typedef SelectableTraits::available_size_type      available_size_type;

      typedef SelectableTraits::InstalledItemSet         InstalledItemSet;
      typedef SelectableTraits::installed_iterator       installed_iterator;
      typedef SelectableTraits::installed_const_iterator installed_const_iterator;
      typedef SelectableTraits::installed_size_type      installed_size_type;

      typedef SelectableTraits::PickList		PickList;

    public:
      template <class _Iterator>
      Impl( const ResObject::Kind & kind_r,
            const std::string & name_r,
            _Iterator begin_r,
            _Iterator end_r )
      : _ident( sat::Solvable::SplitIdent( kind_r, name_r ).ident() )
      , _kind( kind_r )
      , _name( name_r )
      {
        for_( it, begin_r, end_r )
        {
          if ( it->status().isInstalled() )
            _installedItems.insert( *it );
          else
            _availableItems.insert( *it );
        }
      }

    public:
      /**  */
      IdString ident() const
      { return _ident; }

      /**  */
      ResObject::Kind kind() const
      { return _kind; }

      /**  */
      const std::string & name() const
      { return _name; }

      /**  */
      Status status() const;

      /**  */
      bool setStatus( Status state_r, ResStatus::TransactByValue causer_r );

      /** Installed object (transacting ot highest version). */
      PoolItem installedObj() const
      {
        if ( installedEmpty() )
          return PoolItem();
        PoolItem ret( transactingInstalled() );
        return ret ? ret : *_installedItems.begin();
      }

      /** Best among available objects.
       * The transacting candidate or the one scheduled to receive
       * the transact bit.
      */
      PoolItem candidateObj() const
      {
        PoolItem ret( transactingCandidate() );
        if ( ret )
          return ret;
        return _candidate ? _candidate : defaultCandidate();
      }

      /** Set a userCandidate (out of available objects).
       * \return The new userCandidate or NULL if choice was invalid
       * (not among availableObjs).
      */
      PoolItem setCandidate( const PoolItem & newCandidate_r, ResStatus::TransactByValue causer_r );

      /** The best candidate provided by a specific \ref Repository, if there is one.
       * In contrary to \ref candidateObj, this may return no item even if
       * there are available objects. This simply means the \ref Repository
       * does not provide this object.
       */
      PoolItem candidateObjFrom( Repository repo_r ) const
      {
        for_( it, availableBegin(), availableEnd() )
        {
          if ( (*it)->repository() == repo_r )
            return *it;
        }
        return PoolItem();
      }

      /** The best candidate for update, if there is one.
       * In contrary to \ref candidateObj, this may return no item even if
       * there are available objects. This simply means the best object is
       * already installed, and all available objects violate at least one
       * update policy.
       */
      PoolItem updateCandidateObj() const
      {
	PoolItem defaultCand( defaultCandidate() );

	if ( multiversionInstall() )
	  return identicalInstalled( defaultCand ) ? PoolItem() : defaultCand;

        if ( installedEmpty() || ! defaultCand )
          return defaultCand;
        // Here: installed and defaultCand are non NULL and it's not a
        //       multiversion install.

        // update candidate must come from the highest priority repo
        if ( defaultCand->repoInfo().priority() != (*availableBegin())->repoInfo().priority() )
          return PoolItem();

        PoolItem installed( installedObj() );
        // check vendor change
        if ( ! ( ResPool::instance().resolver().allowVendorChange()
                 || VendorAttr::instance().equivalent( defaultCand->vendor(), installed->vendor() ) ) )
          return PoolItem();

        // check arch change (arch noarch changes are allowed)
        if ( defaultCand->arch() != installed->arch()
           && ! ( defaultCand->arch() == Arch_noarch || installed->arch() == Arch_noarch ) )
          return PoolItem();

        // check greater edition
        if ( defaultCand->edition() <= installed->edition() )
          return PoolItem();

        return defaultCand;
      }

      /** \copydoc Selectable::highestAvailableVersionObj()const */
      PoolItem highestAvailableVersionObj() const
      {
        PoolItem ret;
        for_( it, availableBegin(), availableEnd() )
        {
          if ( !ret || (*it).satSolvable().edition() > ret.satSolvable().edition() )
            ret = *it;
        }
        return ret;
      }

      /** \copydoc Selectable::identicalAvailable( const PoolItem & )const */
      bool identicalAvailable( const PoolItem & rhs ) const
      { return bool(identicalAvailableObj( rhs )); }

      /** \copydoc Selectable::identicalInstalled( const PoolItem & )const */
      bool identicalInstalled( const PoolItem & rhs ) const
      { return bool(identicalInstalledObj( rhs )); }

      /** \copydoc Selectable::identicalAvailableObj( const PoolItem & rhs ) const */
      PoolItem identicalAvailableObj( const PoolItem & rhs ) const
      {
        if ( !availableEmpty() && rhs )
        {
          for_( it, _availableItems.begin(), _availableItems.end() )
          {
            if ( identical( *it, rhs ) )
              return *it;
          }
        }
        return PoolItem();
      }

      /** \copydoc Selectable::identicalInstalledObj( const PoolItem & rhs ) const */
      PoolItem identicalInstalledObj( const PoolItem & rhs ) const
      {
        if ( !installedEmpty() && rhs )
        {
          for_( it, _installedItems.begin(), _installedItems.end() )
          {
            if ( identical( *it, rhs ) )
              return *it;
          }
        }
        return PoolItem();
      }

      /** Best among all objects. */
      PoolItem theObj() const
      {
        PoolItem ret( candidateObj() );
        if ( ret )
          return ret;
        return installedObj();
      }

      ////////////////////////////////////////////////////////////////////////

      bool availableEmpty() const
      { return _availableItems.empty(); }

      available_size_type availableSize() const
      { return _availableItems.size(); }

      available_const_iterator availableBegin() const
      { return _availableItems.begin(); }

      available_const_iterator availableEnd() const
      { return _availableItems.end(); }

      ////////////////////////////////////////////////////////////////////////

      bool installedEmpty() const
      { return _installedItems.empty(); }

      installed_size_type installedSize() const
      { return _installedItems.size(); }

      installed_iterator installedBegin() const
      { return _installedItems.begin(); }

      installed_iterator installedEnd() const
      { return _installedItems.end(); }

      ////////////////////////////////////////////////////////////////////////

      const PickList & picklist() const
      {
        if ( ! _picklistPtr )
        {
          _picklistPtr.reset( new PickList );
          // installed without identical avaialble first:
          for_( it, _installedItems.begin(), _installedItems.end() )
          {
            if ( ! identicalAvailable( *it ) )
              _picklistPtr->push_back( *it );
          }
          _picklistPtr->insert( _picklistPtr->end(), availableBegin(), availableEnd() );
        }
        return *_picklistPtr;
      }

      bool picklistEmpty() const
      { return picklist().empty(); }

      picklist_size_type picklistSize() const
      { return picklist().size(); }

      picklist_iterator picklistBegin() const
      { return picklist().begin(); }

      picklist_iterator picklistEnd() const
      { return picklist().end(); }

      ////////////////////////////////////////////////////////////////////////

      bool isUnmaintained() const
      { return availableEmpty(); }

      bool multiversionInstall() const
      { return sat::Pool::instance().isMultiversion( ident() ); }

      bool pickInstall( const PoolItem & pi_r, ResStatus::TransactByValue causer_r, bool yesno_r );

      bool pickDelete( const PoolItem & pi_r, ResStatus::TransactByValue causer_r, bool yesno_r );

      Status pickStatus( const PoolItem & pi_r ) const;

      bool setPickStatus( const PoolItem & pi_r, Status state_r, ResStatus::TransactByValue causer_r );

      ////////////////////////////////////////////////////////////////////////

      bool isUndetermined() const
      {
        PoolItem cand( candidateObj() );
        return ! cand || cand.isUndetermined();
      }
      bool isRelevant() const
      {
        PoolItem cand( candidateObj() );
        return cand && cand.isRelevant();
      }
      bool isSatisfied() const
       {
        PoolItem cand( candidateObj() );
        return cand && cand.isSatisfied();
      }
      bool isBroken() const
      {
        PoolItem cand( candidateObj() );
        return cand && cand.isBroken();
      }

      /** Return who caused the modification. */
      ResStatus::TransactByValue modifiedBy() const;

      /** Return value of LicenceConfirmed bit. */
      bool hasLicenceConfirmed() const
      { return candidateObj() && candidateObj().status().isLicenceConfirmed(); }

      /** Set LicenceConfirmed bit. */
      void setLicenceConfirmed( bool val_r )
      { if ( candidateObj() ) candidateObj().status().setLicenceConfirmed( val_r ); }

    private:
      PoolItem transactingInstalled() const
      {
        for_( it, installedBegin(), installedEnd() )
          {
            if ( (*it).status().transacts() )
              return (*it);
          }
        return PoolItem();
      }

      PoolItem transactingCandidate() const
      {
        for_( it, availableBegin(), availableEnd() )
          {
            if ( (*it).status().transacts() )
              return (*it);
          }
        return PoolItem();
      }

      PoolItem defaultCandidate() const
      {
        if ( ! ( multiversionInstall() || installedEmpty() ) )
        {
          // prefer the installed objects arch and vendor
          bool solver_allowVendorChange( ResPool::instance().resolver().allowVendorChange() );
          for ( installed_const_iterator iit = installedBegin();
                iit != installedEnd(); ++iit )
          {
            PoolItem sameArch; // in case there's no same vendor at least stay with same arch.
            for ( available_const_iterator it = availableBegin();
                  it != availableEnd(); ++it )
            {
              // 'same arch' includes allowed changes to/from noarch.
              if ( (*iit)->arch() == (*it)->arch() || (*iit)->arch() == Arch_noarch || (*it)->arch() == Arch_noarch )
              {
                if ( ! solver_allowVendorChange )
                {
                  if ( VendorAttr::instance().equivalent( (*iit), (*it) ) )
                    return *it;
                  else if ( ! sameArch ) // remember best same arch in case no same vendor found
                     sameArch = *it;
                }
                else // same arch is sufficient
                  return *it;
              }
            }
            if ( sameArch )
              return sameArch;
          }
        }
        if ( _availableItems.empty() )
          return PoolItem();

        return *_availableItems.begin();
      }

      bool allCandidatesLocked() const
      {
        for ( available_const_iterator it = availableBegin();
              it != availableEnd(); ++it )
          {
            if ( ! (*it).status().isLocked() )
              return false;
          }
        return( ! _availableItems.empty() );
      }

      bool allInstalledLocked() const
      {
        for ( installed_const_iterator it = installedBegin();
              it != installedEnd(); ++it )
          {
            if ( ! (*it).status().isLocked() )
              return false;
          }
        return( ! _installedItems.empty() );
      }


    private:
      const IdString         _ident;
      const ResObject::Kind  _kind;
      const std::string      _name;
      InstalledItemSet       _installedItems;
      AvailableItemSet       _availableItems;
      //! The object selected by setCandidateObj() method.
      PoolItem               _candidate;
      //! lazy initialized picklist
      mutable scoped_ptr<PickList> _picklistPtr;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Selectable::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const Selectable::Impl & obj )
    {
      return str << '[' << obj.kind() << ']' << obj.name() << ": " << obj.status()
                 << " (I " << obj.installedSize() << ")"
                 << " (A " << obj.availableSize() << ")"
                 << obj.candidateObj();
    }

    /** \relates Selectable::Impl Stream output */
    inline std::ostream & dumpOn( std::ostream & str, const Selectable::Impl & obj )
    {
      str << '[' << obj.kind() << ']' << obj.name() << ": " << obj.status()
          << ( obj.multiversionInstall() ? " (multiversion)" : "") << endl;

      if ( obj.installedEmpty() )
        str << "   (I 0) {}" << endl << "   ";
      else
      {
        PoolItem icand( obj.installedObj() );
        str << "   (I " << obj.installedSize() << ") {" << endl;
        for_( it, obj.installedBegin(), obj.installedEnd() )
        {
          char t = ' ';
          if ( *it == icand )
          {
            t = 'i';
          }
          str << " " << t << " " << *it << endl;
        }
        str << "}  ";
      }

      if ( obj.availableEmpty() )
      {
        str << "(A 0) {}" << endl << "   ";
      }
      else
      {
        PoolItem cand( obj.candidateObj() );
        PoolItem up( obj.updateCandidateObj() );
        str << "(A " << obj.availableSize() << ") {" << endl;
        for_( it, obj.availableBegin(), obj.availableEnd() )
        {
          char t = ' ';
          if ( *it == cand )
          {
            t = *it == up ? 'C' : 'c';
          }
          else if ( *it == up )
          {
            t = 'u';
          }
          str << " " << t << " " << *it << endl;
        }
        str << "}  ";
      }

      if ( obj.picklistEmpty() )
      {
        str << "(P 0) {}";
      }
      else
      {
        PoolItem cand( obj.candidateObj() );
        PoolItem up( obj.updateCandidateObj() );
        str << "(P " << obj.picklistSize() << ") {" << endl;
        for_( it, obj.picklistBegin(), obj.picklistEnd() )
        {
          char t = ' ';
          if ( *it == cand )
          {
            t = *it == up ? 'C' : 'c';
          }
          else if ( *it == up )
          {
            t = 'u';
          }
          str << " " << t << " " << *it << "\t" << obj.pickStatus( *it ) << endl;
        }
        str << "}  ";
      }

      return str;
    }
    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_UI_SELECTABLEIMPL_H
