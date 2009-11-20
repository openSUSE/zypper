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

#include "zypp/ZConfig.h"
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
        _defaultCandidate = defaultCandidate();
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
      bool setStatus( const Status state_r, ResStatus::TransactByValue causer_r );

      /** Installed object. */
      PoolItem installedObj() const
      {
          if (!installedEmpty())
              return *_installedItems.begin();
          return PoolItem();
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
        return _candidate ? _candidate : _defaultCandidate;
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
        if ( installedEmpty() || ! _defaultCandidate )
          return _defaultCandidate;
        // Here: installed and _defaultCandidate are non NULL.

        // update candidate must come from the highest priority repo
        if ( _defaultCandidate->repoInfo().priority() != (*availableBegin())->repoInfo().priority() )
          return PoolItem();

        PoolItem installed( installedObj() );
        // check vendor change
        if ( ! ( ZConfig::instance().solver_allowVendorChange()
                 || VendorAttr::instance().equivalent( _defaultCandidate->vendor(), installed->vendor() ) ) )
          return PoolItem();

        // check arch change (arch noarch changes are allowed)
        if ( _defaultCandidate->arch() != installed->arch()
           && ! ( _defaultCandidate->arch() == Arch_noarch || installed->arch() == Arch_noarch ) )
          return PoolItem();

        // check greater edition
        if ( _defaultCandidate->edition() <= installed->edition() )
          return PoolItem();

        return _defaultCandidate;
      }

      /** \c True if \a rhs has the same content as an installed one.
       * \see \ref sat::Solvable::identical
       */
      bool identicalInstalled( const PoolItem & rhs ) const
      {
        if ( !installedEmpty() && rhs )
        {
          for_( it, _installedItems.begin(), _installedItems.end() )
          {
            if ( identical( *it, rhs ) )
              return true;
          }
        }
        return false;
      }

      /** Best among all objects. */
      PoolItem theObj() const
      {
        PoolItem ret( candidateObj() );
        if (ret)
            return ret;

        if ( ! _installedItems.empty() )
            return  (*_installedItems.begin());

        return PoolItem();
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

      bool isUnmaintained() const
      { return availableEmpty(); }

      bool multiversionInstall() const
      { return theObj().satSolvable().multiversionInstall(); }

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
      PoolItem transactingCandidate() const
      {
        for ( available_const_iterator it = availableBegin();
              it != availableEnd(); ++it )
          {
            if ( (*it).status().transacts() )
              return (*it);
          }
        return PoolItem();
      }

      PoolItem defaultCandidate() const
      {
        if ( !installedEmpty() )
        {
          // prefer the installed objects arch and vendor
          bool solver_allowVendorChange( ZConfig::instance().solver_allowVendorChange() );
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

    private:
      const IdString         _ident;
      const ResObject::Kind  _kind;
      const std::string      _name;
      InstalledItemSet       _installedItems;
      AvailableItemSet       _availableItems;
      //! Best among availabe with restpect to installed.
      PoolItem               _defaultCandidate;

      //! The object selected by setCandidateObj() method.
      PoolItem               _candidate;
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
        str << "   (I " << obj.installedSize() << ") {" << endl;
        for_( it, obj.installedBegin(), obj.installedEnd() )
        {
          str << "   " << *it << endl;
        }
        str << "}  ";
      }

      if ( obj.availableEmpty() )
      {
        str << "(A 0) {}";
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

      return str;
    }
    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_UI_SELECTABLEIMPL_H
