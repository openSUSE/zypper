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

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResStatus
  //
  /** Status bitfield.
   *
   * \li \c StateField Whether the resolvable is currently installed
   *        or uninstalled (available).
   * \li \c EstablishField Established status computed by the solver as
   *        unneeded (have freshens but none of them trigger)
   *	    satisfied (no freshen or at least one triggered freshen and
   *	    all requires fulfilled)
   *	    or incomplete (no freshen or at least one triggered freshen and
   *	    NOT all requires fulfilled)
   * \li \c TransactField Wheter to transact this resolvable
   *        (delete if installed install if uninstalled).
   * \li \c TransactByField Who triggered the transaction. Transaction
   *        bit may be reset by higer levels only.
   * \li \c TransactDetailField Reason why the Resolvable transacts.
   *        Splitted into \c InstallDetailValue and \c RemoveDetailValue
   *        dependent on the kind of transaction.
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
    typedef short FieldType;
    typedef bit::BitField<FieldType> BitFieldType;
    // Bit Ranges within FieldType defined by 1st bit and size:
    typedef bit::Range<FieldType,0,                    1> StateField;
    typedef bit::Range<FieldType,StateField::end,      2> EstablishField;
    typedef bit::Range<FieldType,EstablishField::end,  1> TransactField;
    typedef bit::Range<FieldType,TransactField::end,   2> TransactByField;
    typedef bit::Range<FieldType,TransactByField::end, 3> TransactDetailField;
    typedef bit::Range<FieldType,TransactDetailField::end, 2> SolverStateField;
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
    enum EstablishValue
      {
        UNDETERMINED = bit::RangeValue<EstablishField,0>::value,
        UNNEEDED     = bit::RangeValue<EstablishField,1>::value, // has freshens, none trigger
        SATISFIED    = bit::RangeValue<EstablishField,2>::value, // has none or triggered freshens, all requirements fulfilled
        INCOMPLETE   = bit::RangeValue<EstablishField,3>::value	 // installed: has none or triggered freshens, requirements unfulfilled
      };
    enum TransactValue
      {
        KEEP_STATE = bit::RangeValue<TransactField,0>::value,
        TRANSACT   = bit::RangeValue<TransactField,1>::value // change state installed <-> uninstalled
      };
    enum TransactByValue
      {
        SOLVER    = bit::RangeValue<TransactByField,0>::value,
        APPL_LOW  = bit::RangeValue<TransactByField,1>::value,
        APPL_HIGH = bit::RangeValue<TransactByField,2>::value,
        USER      = bit::RangeValue<TransactByField,3>::value
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
        DUE_TO_UNLINK   = bit::RangeValue<TransactDetailField,3>::value,
        DUE_TO_UPGRADE  = bit::RangeValue<TransactDetailField,4>::value
      };
    enum SolverStateValue
      {
	NORMAL     = bit::RangeValue<SolverStateField,0>::value, // default, notthing special
	SEEN       = bit::RangeValue<SolverStateField,1>::value, // already seen during ResolverUpgrade
	IMPOSSIBLE = bit::RangeValue<SolverStateField,2>::value	 // impossible to install
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

    bool isUndetermined() const
    { return fieldValueIs<EstablishField>( UNDETERMINED ); }

    bool isUnneeded() const
    { return isUninstalled() && fieldValueIs<EstablishField>( UNNEEDED ); }

    bool isSatisfied() const
    { return isUninstalled() && fieldValueIs<EstablishField>( SATISFIED ); }

    bool isComplete () const
    { return isInstalled() && fieldValueIs<EstablishField>( SATISFIED ); }

    bool isIncomplete() const
    { return isInstalled() && fieldValueIs<EstablishField>( INCOMPLETE ); }

    bool isNeeded() const
    { return isUninstalled() && fieldValueIs<EstablishField>( INCOMPLETE ); }

    bool transacts() const
    { return fieldValueIs<TransactField>( TRANSACT ); }

    bool isBySolver() const
    { return fieldValueIs<TransactByField>( SOLVER ); }

    bool isByApplLow() const
    { return fieldValueIs<TransactByField>( APPL_LOW ); }

    bool isByApplHigh() const
    { return fieldValueIs<TransactByField>( APPL_HIGH ); }

    bool isByUser() const
    { return fieldValueIs<TransactByField>( USER ); }

    bool isToBeUninstalledDueToObsolete () const
    { return isToBeUninstalled() && fieldValueIs<TransactDetailField>( DUE_TO_OBSOLETE ); }

    bool isToBeUninstalledDueToUnlink() const
    { return isToBeUninstalled() && fieldValueIs<TransactDetailField>( DUE_TO_UNLINK ); }

    bool isToBeUninstalledDueToUpgrade() const
    { return isToBeUninstalled() && fieldValueIs<TransactDetailField>( DUE_TO_UPGRADE ); }

    bool isToBeInstalledSoft () const
    { return isToBeInstalled() && fieldValueIs<TransactDetailField>( SOFT_INSTALL ); }

    bool isToBeUninstalledSoft () const
    { return isToBeUninstalled() && fieldValueIs<TransactDetailField>( SOFT_REMOVE ); }

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

    bool setTransact (bool val_r, TransactByValue causer)
    {
	if (!isGreaterThan<TransactByField>( causer )) {
	    fieldValueAssign<TransactField>( val_r ? TRANSACT : KEEP_STATE );
	    fieldValueAssign<TransactByField>( causer );			// remember the causer
	    return true;
	} else if (fieldValueIs<TransactField>(val_r ? TRANSACT : KEEP_STATE)) {
	    return true; // is already set
	} else {
	    return false;
	}
    }

    bool maySetTransact (bool val_r, TransactByValue causer)
    {
	bit::BitField<FieldType> savBitfield = _bitfield;
	bool ret = setTransact (val_r, causer);
	_bitfield = savBitfield;
	return ret;
    }

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

    bool setToBeUninstalledDueToUnlink ( )
    {
      if (!setToBeUninstalled (SOLVER)) return false;
      fieldValueAssign<TransactDetailField>(DUE_TO_UNLINK);
      return true;
    }

    bool setToBeUninstalledDueToObsolete ( )
    {
      if (!setToBeUninstalled (SOLVER)) return false;
      fieldValueAssign<TransactDetailField>(DUE_TO_OBSOLETE);
      return true;
    }

    bool setToBeUninstalledDueToUpgrade ( )
    {
      if (!setToBeUninstalled (SOLVER)) return false;
      fieldValueAssign<TransactDetailField>(DUE_TO_UPGRADE);
      return true;
    }

    bool setToBeInstalledSoft ( )
    {
      if (!setToBeInstalled(SOLVER)) return false;
      fieldValueAssign<TransactDetailField>(SOFT_INSTALL);
      return true;
    }

    bool setToBeUninstalledSoft ( )
    {
      if (!setToBeUninstalled(SOLVER)) return false;
      fieldValueAssign<TransactDetailField>(SOFT_REMOVE);
      return true;
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
      fieldValueAssign<EstablishField>(UNDETERMINED);
      return true;
    }

    bool setUnneeded ()
    {
      fieldValueAssign<EstablishField>(UNNEEDED);
      return true;
    }

    bool setSatisfied ()
    {
      fieldValueAssign<EstablishField>(SATISFIED);
      return true;
    }

    bool setIncomplete ()
    {
      fieldValueAssign<EstablishField>(INCOMPLETE);
      return true;
    }

    bool isSeen () const
    { return fieldValueIs<SolverStateField>( SEEN ); }

    bool isImpossible () const
    { return fieldValueIs<SolverStateField>( IMPOSSIBLE ); }

    bool setSeen (bool value)
    {
      fieldValueAssign<SolverStateField>( value ? SEEN : NORMAL );
      return true;
    }

    bool setImpossible (bool value)
    {
      fieldValueAssign<SolverStateField>( value ? IMPOSSIBLE : NORMAL );
      return true;
    }

    bool setStatus (ResStatus status)
    {
	if ( ((isInstalled() && status.isInstalled())
	       || (isUninstalled() && status.isUninstalled()))
	     && _bitfield.value<TransactByField>() <=  status._bitfield.value<TransactByField>() // regarding priority
	     )
	{
	    _bitfield = status._bitfield;
	    return true;
	}
	else
	{
	    // The type is not correct ( system/source)
	    // or has not enough priority
	    return false;
	}
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
    static const ResStatus impossible;	// uninstallable
    //@}

  private:
    /** Ctor for intialization of builtin constants. */
    ResStatus( StateValue s,
               EstablishValue e     = UNDETERMINED,
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
