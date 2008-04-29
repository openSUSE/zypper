/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResStatus.h
 *
*/
#ifndef ZYPP_RESSTATUS_H
#define ZYPP_RESSTATUS_H

#include <iosfwd>

#include "zypp/Bit.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace resstatus
  {
    class UserLockQueryManip;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResStatus
  //
  /** Status bitfield.
   *
   * \li \c StateField Whether the resolvable is or uninstalled (available).
   * \li \c ValidateField Validate status computed by the solver as
   *        nonrelevant: it is unimportant for the user
   *	    satisfied: it important nothing has to be done
   *	    broken: it is incomplete. So e.g. an update is needed
   * \li \c TransactField Wheter to transact this resolvable
   *        (delete if installed install if uninstalled).
   *        In case the resolvable is locked, only USER may modify the
   *        transact bit.
   * \li \c TransactByField Who triggered the transaction. Transaction
   *        bit may be reset by higer levels only.
   * \li \c TransactDetailField Reason why the Resolvable transacts.
   *        Splitted into \c InstallDetailValue and \c RemoveDetailValue
   *        dependent on the kind of transaction.
   * \li \c WeakField The solvable will be recommended/suggested by
   *        a to be installed/deleted solvable.
   *
  */
  class ResStatus
  {
    friend std::ostream & operator<<( std::ostream & str, const ResStatus & obj );
    friend bool operator==( const ResStatus & lhs, const ResStatus & rhs );

  public:
    /** \name BitField range definitions.
     *
     * \note Enlarge FieldType if more bit's needed. It's not yet
     * checked by the compiler.
     */
    //@{
    typedef uint16_t FieldType;
    typedef bit::BitField<FieldType> BitFieldType;
    // Bit Ranges within FieldType defined by 1st bit and size:
    typedef bit::Range<FieldType,0,                          1> StateField;
    typedef bit::Range<FieldType,StateField::end,            2> ValidateField;
    typedef bit::Range<FieldType,ValidateField::end,         2> TransactField;
    typedef bit::Range<FieldType,TransactField::end,         2> TransactByField;
    typedef bit::Range<FieldType,TransactByField::end,       2> TransactDetailField;
    typedef bit::Range<FieldType,TransactDetailField::end,   1> SolverStateField;
    typedef bit::Range<FieldType,SolverStateField::end,      1> LicenceConfirmedField;
    typedef bit::Range<FieldType,LicenceConfirmedField::end, 2> WeakField;
    typedef bit::Range<FieldType,WeakField::end,             1> UserLockQueryField; // internal
    // enlarge FieldType if more bit's needed. It's not yet
    // checked by the compiler.
    //@}
  public:

    /** \name Status values.
     *
     * Each enum corresponds to a BitField range.
     * \note Take care that enumerator values actually fit into
     * the corresponding field. It's not yet checked by the compiler.
     */
    //@{
    enum StateValue
      {
        UNINSTALLED = bit::RangeValue<StateField,0>::value,
        INSTALLED   = bit::RangeValue<StateField,1>::value
      };
    enum ValidateValue
      {
	UNDETERMINED = bit::RangeValue<ValidateField,0>::value,
        BROKEN       = bit::RangeValue<ValidateField,1>::value,
        SATISFIED    = bit::RangeValue<ValidateField,2>::value,
        NONRELEVANT  = bit::RangeValue<ValidateField,3>::value
      };
    enum TransactValue
      {
        KEEP_STATE = bit::RangeValue<TransactField,0>::value,
        LOCKED     = bit::RangeValue<TransactField,1>::value, // locked, must not transact
        TRANSACT   = bit::RangeValue<TransactField,2>::value  // transact according to state
      };
    enum TransactByValue
      {
        SOLVER    = bit::RangeValue<TransactByField,0>::value,
        APPL_LOW  = bit::RangeValue<TransactByField,1>::value,
        APPL_HIGH = bit::RangeValue<TransactByField,2>::value,
        USER      = bit::RangeValue<TransactByField,3>::value
      };

    enum DetailValue
      {
        /** Detail for no transact, i.e. reset any Install/RemoveDetailValue. */
        NO_DETAIL = bit::RangeValue<TransactDetailField,0>::value,
      };

    enum InstallDetailValue
      {
        EXPLICIT_INSTALL = bit::RangeValue<TransactDetailField,0>::value,
        SOFT_INSTALL     = bit::RangeValue<TransactDetailField,1>::value
      };
    enum RemoveDetailValue
      {
        EXPLICIT_REMOVE = bit::RangeValue<TransactDetailField,0>::value,
	SOFT_REMOVE     = bit::RangeValue<TransactDetailField,1>::value,
        DUE_TO_OBSOLETE = bit::RangeValue<TransactDetailField,2>::value,
        DUE_TO_UPGRADE  = bit::RangeValue<TransactDetailField,3>::value
      };
    enum SolverStateValue
      {
	NORMAL     = bit::RangeValue<SolverStateField,0>::value, // default, notthing special
	SEEN       = bit::RangeValue<SolverStateField,1>::value, // already seen during ResolverUpgrade
      };

    enum LicenceConfirmedValue
      {
        LICENCE_UNCONFIRMED = bit::RangeValue<LicenceConfirmedField,0>::value,
        LICENCE_CONFIRMED   = bit::RangeValue<LicenceConfirmedField,1>::value
      };

    enum WeakValue
      {
        NO_WEAK 		= bit::RangeValue<WeakField,0>::value,
        SUGGESTED   		= bit::RangeValue<WeakField,1>::value,
	RECOMMENDED 		= bit::RangeValue<WeakField,2>::value,
	SUGGESTED_AND_RECOMMENDED = bit::RangeValue<WeakField,3>::value
      };

    enum UserLockQuery // internal
      {
        USERLOCK_NOMATCH	= bit::RangeValue<UserLockQueryField,0>::value,
        USERLOCK_MATCH		= bit::RangeValue<UserLockQueryField,1>::value
      };
    //@}

  public:

    /** Default ctor. */
    ResStatus();

    /** Ctor setting the initial . */
    ResStatus( bool isInstalled_r );

    /** Dtor. */
    ~ResStatus();

    /** Debug helper returning the bitfield.
     * It's save to expose the bitfield, as it can't be used to
     * recreate a ResStatus. So it is not possible to bypass
     * transition rules.
    */
    BitFieldType bitfield() const
    { return _bitfield; }

  public:

    bool isLicenceConfirmed() const
    { return fieldValueIs<LicenceConfirmedField>( LICENCE_CONFIRMED ); }

    void setLicenceConfirmed( bool toVal_r = true )
    { fieldValueAssign<LicenceConfirmedField>( toVal_r ? LICENCE_CONFIRMED : LICENCE_UNCONFIRMED ); }

  public:

    bool isRecommended() const
    { return fieldValueIs<WeakField>( RECOMMENDED ); }

    bool isSuggested() const
    { return fieldValueIs<WeakField>( SUGGESTED ); }

    bool resetWeak() const
    { return fieldValueIs<WeakField>( NO_WEAK ); }

    void setRecommended( bool toVal_r = true )
    { if (isSuggested())
	fieldValueAssign<WeakField>( toVal_r ? SUGGESTED_AND_RECOMMENDED : SUGGESTED );
    else
	fieldValueAssign<WeakField>( toVal_r ? RECOMMENDED : NO_WEAK );
    }

    void setSuggested( bool toVal_r = true )
    { if (isRecommended())
	fieldValueAssign<WeakField>( toVal_r ? SUGGESTED_AND_RECOMMENDED : RECOMMENDED );
    else
	fieldValueAssign<WeakField>( toVal_r ? SUGGESTED : NO_WEAK );
    }

    ValidateValue validate() const
    { return (ValidateValue)_bitfield.value<ValidateField>(); }

    bool isUndetermined() const
    { return fieldValueIs<ValidateField>( UNDETERMINED ); }

    bool isSatisfied() const
    { return fieldValueIs<ValidateField>( SATISFIED ); }

    bool isBroken() const
    { return fieldValueIs<ValidateField>( BROKEN ); }

    bool isNonRelevant() const
    { return fieldValueIs<ValidateField>( NONRELEVANT ); }

  public:
    // These two are IMMUTABLE!

    bool isInstalled() const
    { return fieldValueIs<StateField>( INSTALLED ); }

    bool isUninstalled() const
    { return fieldValueIs<StateField>( UNINSTALLED ); }

  public:

    bool staysInstalled() const
    { return isInstalled() && !transacts(); }

    bool wasInstalled() const { return staysInstalled(); }	//for old status

    bool isToBeInstalled() const
    { return isUninstalled() && transacts(); }

    bool staysUninstalled() const
    { return isUninstalled() && !transacts(); }

    bool wasUninstalled() const { return staysUninstalled(); }	// for old status

    bool isToBeUninstalled() const
    { return isInstalled() && transacts(); }

    bool isLocked() const
    { return fieldValueIs<TransactField>( LOCKED ); }

    bool isSoftLocked( TransactByValue causer_r = USER ) const
    { return isKept() && fieldValueIs<TransactByField>( causer_r ); }

    bool isKept() const
    { return fieldValueIs<TransactField>( KEEP_STATE ); }

    bool transacts() const
    { return fieldValueIs<TransactField>( TRANSACT ); }

    TransactValue getTransactValue() const
    { return (TransactValue)_bitfield.value<TransactField>(); }

    bool isBySolver() const
    { return fieldValueIs<TransactByField>( SOLVER ); }

    bool isByApplLow() const
    { return fieldValueIs<TransactByField>( APPL_LOW ); }

    bool isByApplHigh() const
    { return fieldValueIs<TransactByField>( APPL_HIGH ); }

    bool isByUser() const
    { return fieldValueIs<TransactByField>( USER ); }

    TransactByValue getTransactByValue() const
    { return (TransactByValue)_bitfield.value<TransactByField>(); }


    bool isToBeUninstalledDueToObsolete () const
    { return isToBeUninstalled() && fieldValueIs<TransactDetailField>( DUE_TO_OBSOLETE ); }

    bool isToBeUninstalledDueToUpgrade() const
    { return isToBeUninstalled() && fieldValueIs<TransactDetailField>( DUE_TO_UPGRADE ); }

    bool isToBeInstalledSoft () const
    { return isToBeInstalled() && fieldValueIs<TransactDetailField>( SOFT_INSTALL ); }

    bool isToBeInstalledNotSoft () const
    { return isToBeInstalled() && !fieldValueIs<TransactDetailField>( SOFT_INSTALL ); }

    bool isToBeUninstalledSoft () const
    { return isToBeUninstalled() && fieldValueIs<TransactDetailField>( SOFT_REMOVE ); }

  private:

    /** \name Internal hard lock maintainance */
    //@{
    friend class resstatus::UserLockQueryManip;

    bool isUserLockQueryMatch() const
    { return fieldValueIs<UserLockQueryField>( USERLOCK_MATCH ); }

    void setUserLockQueryMatch( bool match_r )
    { fieldValueAssign<UserLockQueryField>( match_r ? USERLOCK_MATCH : USERLOCK_NOMATCH ); }
    //@}

  public:

    //------------------------------------------------------------------------
    // get/set functions, returnig \c true if requested status change
    // was successfull (i.e. leading to the desired transaction).
    // If a lower level (e.g.SOLVER) wants to transact, but it's
    // already set by a higher level, \c true should be returned.
    // Removing a higher levels transaction bit should fail.
    //
    // The may functions checks only, if the action would return true
    // if it is called.

    /** Set TransactValue.
     * Convenience to set TransactValue from enum.
     */
    bool setTransactValue( TransactValue newVal_r, TransactByValue causer_r )
    {
      switch ( newVal_r )
        {
        case KEEP_STATE:
          return setTransact( false, causer_r );
          break;
        case LOCKED:
          return setLock( true, causer_r );
          break;
        case TRANSACT:
          return setTransact( true, causer_r );
          break;
        }
      return false;
    }

    bool maySetTransactValue( TransactValue newVal_r, TransactByValue causer_r )
    {
	bit::BitField<FieldType> savBitfield = _bitfield;
	bool ret = setTransactValue( newVal_r, causer_r );
	_bitfield = savBitfield;
	return ret;
    }

    /** Apply a lock (prevent transaction).
     * Currently by USER only, but who knows... Set LOCKED
     * from KEEP_STATE to be shure all transaction details
     * were reset properly.
    */
    bool setLock( bool toLock_r, TransactByValue causer_r )
    {
      if ( toLock_r == isLocked() )
        {
          // we're already in the desired state, but in case of
          // LOCKED, remember a superior causer.
          if ( isLocked() && isLessThan<TransactByField>( causer_r ) )
            fieldValueAssign<TransactByField>( causer_r );
           return true;
        }
      // Here: Lock status is to be changed:
      if ( causer_r != USER )
        return false;
      // Setting no transact removes an existing lock,
      // or brings this into KEEP_STATE, and we apply the lock.
      if ( ! setTransact( false, causer_r ) )
        return false;
      if ( toLock_r ) {
	  fieldValueAssign<TransactField>( LOCKED );
	  fieldValueAssign<TransactByField>( causer_r );
      } else {
	  fieldValueAssign<TransactField>( KEEP_STATE );
	  fieldValueAssign<TransactByField>( SOLVER ); // reset to lowest causer
	                                               // in order to distinguish from keep_state_by_user
      }
      return true;
    }

    bool maySetLock( bool to_r, TransactByValue causer_r )
    {
	bit::BitField<FieldType> savBitfield = _bitfield;
	bool ret = setLock( to_r, causer_r );
	_bitfield = savBitfield;
	return ret;
    }

    /** Toggle between TRANSACT and KEEP_STATE.
     * LOCKED state means KEEP_STATE. But in contrary to KEEP_STATE,
     * LOCKED state is immutable for \a causer_r less than TransactByValue.
     * KEEP_STATE may be canged by any \a causer_r.
    */
    bool setTransact( bool toTansact_r, TransactByValue causer_r )
    {
      if ( toTansact_r == transacts() )
        {
          // we're already in the desired state, but in case of
          // TRANSACT, remember a superior causer.
          if ( transacts() && isLessThan<TransactByField>( causer_r ) )
            {
              fieldValueAssign<TransactByField>( causer_r );
              // ??? adapt TransactDetailField ?
            }
          return true;
        }
      // Here: transact status is to be changed:
      if (    ! fieldValueIs<TransactField>( KEEP_STATE )
           && isGreaterThan<TransactByField>( causer_r ) )
        return false;

      if ( toTansact_r )
        {
          fieldValueAssign<TransactField>( TRANSACT );
          // ??? adapt TransactDetailField ?
        }
      else
        {
          fieldValueAssign<TransactField>( KEEP_STATE );
          fieldValueAssign<TransactDetailField>( NO_DETAIL );
        }
      fieldValueAssign<TransactByField>( causer_r );
      return true;
    }

    bool maySetTransact( bool val_r, TransactByValue causer )
    {
	bit::BitField<FieldType> savBitfield = _bitfield;
	bool ret = setTransact (val_r, causer);
	_bitfield = savBitfield;
	return ret;
    }

    /** */
    bool setSoftLock( TransactByValue causer_r )
    {
      if ( ! setTransact( false, causer_r ) )
        return false;
      if ( fieldValueIs<TransactField>( KEEP_STATE )
           && isLessThan<TransactByField>( causer_r ) )
        fieldValueAssign<TransactByField>( causer_r );
      return true;
    }

    /** Not the same as setTransact( false ).
     */
    bool resetTransact( TransactByValue causer_r )
    {
      if ( ! setTransact( false, causer_r ) )
        return false;
      if ( fieldValueIs<TransactField>( KEEP_STATE ) )
        fieldValueAssign<TransactByField>( SOLVER );
      return true;
    }

    /** Soft toggle between TRANSACT and KEEP_STATE.
     * Similar to setTransact, but leaving KEEP_STATE also requires
     * a superior \a causerLimit_r. So this is a kind of soft lock.
     * \code
     * // SOLVER wants to set TRANSACT, iff KEEP_STATE is
     * // not superior to APPL_LOW.
     * setSoftTransact( true, SOLVER, APPL_LOW );
     * \endcode
    */
    bool setSoftTransact( bool toTansact_r, TransactByValue causer_r,
                          TransactByValue causerLimit_r )
    {
      if ( fieldValueIs<TransactField>( KEEP_STATE )
           && toTansact_r != transacts()
           && isGreaterThan<TransactByField>( causerLimit_r ) )
        {
          // any transact status change requires a superior causer.
          return false;
        }
      return setTransact( toTansact_r, causer_r );
    }

    bool setSoftTransact( bool toTansact_r, TransactByValue causer_r )
    { return setSoftTransact( toTansact_r, causer_r, causer_r ); }

    bool maySetSoftTransact( bool val_r, TransactByValue causer,
                             TransactByValue causerLimit_r )
    {
	bit::BitField<FieldType> savBitfield = _bitfield;
	bool ret = setSoftTransact( val_r, causer, causerLimit_r );
	_bitfield = savBitfield;
	return ret;
    }

    bool maySetSoftTransact( bool val_r, TransactByValue causer )
    { return maySetSoftTransact( val_r, causer, causer ); }

    bool setToBeInstalled (TransactByValue causer)
    {
      if (isInstalled()) return false;
      return setTransact (true, causer);
    }

    bool maySetToBeInstalled (TransactByValue causer)
    {
	bit::BitField<FieldType> savBitfield = _bitfield;
	bool ret = setToBeInstalled (causer);
	_bitfield = savBitfield;
	return ret;
    }

    bool setToBeUninstalled (TransactByValue causer)
    {
      if (!isInstalled()) return false;
      return setTransact (true, causer);
    }

    bool maySetToBeUninstalled (TransactByValue causer)
    {
	bit::BitField<FieldType> savBitfield = _bitfield;
	bool ret = setToBeUninstalled (causer);
	_bitfield = savBitfield;
	return ret;
    }

    //------------------------------------------------------------------------
    // *** These are only for the Resolver ***

    bool setToBeUninstalledDueToObsolete ( )
    {
      if (!setToBeUninstalled (SOLVER)) return false;
      fieldValueAssign<TransactDetailField>(DUE_TO_OBSOLETE);
      return true;
    }

    bool setToBeUninstalledDueToUpgrade ( TransactByValue causer )
    {
      if (!setToBeUninstalled (causer)) return false;
      fieldValueAssign<TransactDetailField>(DUE_TO_UPGRADE);
      return true;
    }

    bool setToBeInstalledSoft ( )
    {
      if (isInstalled()
	  || !setSoftTransact (true, SOLVER))
	  return false;

      fieldValueAssign<TransactDetailField>(SOFT_INSTALL);
      return true;
    }

    bool setToBeUninstalledSoft ( )
    {
      if (!isInstalled()
	  || !setSoftTransact (true, SOLVER))
	  return false;

      fieldValueAssign<TransactDetailField>(SOFT_REMOVE);
      return true;
    }

    bool maySetToBeUninstalledSoft ()
    {
	bit::BitField<FieldType> savBitfield = _bitfield;
	bool ret = setToBeUninstalledSoft ();
	_bitfield = savBitfield;
	return ret;
    }

    bool isSoftInstall () {
        return fieldValueIs<TransactDetailField> (SOFT_INSTALL);
    }

    bool isSoftUninstall () {
        return fieldValueIs<TransactDetailField> (SOFT_REMOVE);
    }

    bool setSoftInstall (bool flag) {
        fieldValueAssign<TransactDetailField>(flag?SOFT_INSTALL:0);
	return true;
    }

    bool setSoftUninstall (bool flag) {
        fieldValueAssign<TransactDetailField>(flag?SOFT_REMOVE:0);
	return true;
    }

    bool setUndetermined ()
    {
      fieldValueAssign<ValidateField>(UNDETERMINED);
      return true;
    }

    bool setSatisfied ()
    {
      fieldValueAssign<ValidateField>(SATISFIED);
      return true;
    }

    bool setBroken ()
    {
      fieldValueAssign<ValidateField>(BROKEN);
      return true;
    }

    bool setNonRelevant ()
    {
      fieldValueAssign<ValidateField>(NONRELEVANT);
      return true;
    }

    bool isSeen () const
    { return fieldValueIs<SolverStateField>( SEEN ); }

    bool setSeen (bool value)
    {
      fieldValueAssign<SolverStateField>( value ? SEEN : NORMAL );
      return true;
    }

    bool setStatus( ResStatus newStatus_r )
    {
      // State field is immutable!
      if ( _bitfield.value<StateField>() != newStatus_r._bitfield.value<StateField>() )
        return false;
      // Transaction state change allowed?
      if ( ! setTransactValue( newStatus_r.getTransactValue(), newStatus_r.getTransactByValue() ) )
        return false;

      // Ok, we take it all..
      _bitfield = newStatus_r._bitfield;
      return true;
    }

    /** \name Builtin ResStatus constants. */
    //@{
    static const ResStatus toBeInstalled;
    static const ResStatus toBeInstalledSoft;
    static const ResStatus toBeUninstalled;
    static const ResStatus toBeUninstalledSoft;
    static const ResStatus toBeUninstalledDueToUnlink;
    static const ResStatus toBeUninstalledDueToObsolete;
    static const ResStatus toBeUninstalledDueToUpgrade;
    static const ResStatus installed;	// installed, status after successful target 'install' commit
    static const ResStatus uninstalled;	// uninstalled, status after successful target 'uninstall' commit
    static const ResStatus satisfied;	// uninstalled, satisfied
    static const ResStatus complete;	// installed, satisfied
    static const ResStatus unneeded;	// uninstalled, unneeded
    static const ResStatus needed;	// uninstalled, incomplete
    static const ResStatus incomplete;	// installed, incomplete
    static const ResStatus recommended;	// recommended
    static const ResStatus suggested;	// suggested
    //@}

  private:
    /** Ctor for intialization of builtin constants. */
    ResStatus( StateValue s,
               ValidateValue v      = UNDETERMINED,
               TransactValue t      = KEEP_STATE,
               InstallDetailValue i = EXPLICIT_INSTALL,
               RemoveDetailValue r  = EXPLICIT_REMOVE,
	       SolverStateValue ssv = NORMAL );

    /** Return whether the corresponding Field has value \a val_r.
    */
    template<class _Field>
      bool fieldValueIs( FieldType val_r ) const
      { return _bitfield.isEqual<_Field>( val_r ); }

    /** Set the corresponding Field to value \a val_r.
    */
    template<class _Field>
      void fieldValueAssign( FieldType val_r )
      { _bitfield.assign<_Field>( val_r ); }

    /** compare two values.
    */
    template<class _Field>
      bool isGreaterThan( FieldType val_r )
	  { return _bitfield.value<_Field>() > val_r; }

    template<class _Field>
      bool isLessThan( FieldType val_r )
	  { return _bitfield.value<_Field>() < val_r; }

  private:
    BitFieldType _bitfield;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ResStatus Stream output */
  std::ostream & operator<<( std::ostream & str, const ResStatus & obj );

  /** \relates ResStatus */
  inline bool operator==( const ResStatus & lhs, const ResStatus & rhs )
  { return lhs._bitfield == rhs._bitfield; }

  /** \relates ResStatus */
  inline bool operator!=( const ResStatus & lhs, const ResStatus & rhs )
  { return ! (lhs == rhs); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESSTATUS_H
