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
  */
  class ResStatus
  {
    friend std::ostream & operator<<( std::ostream & str, const ResStatus & obj );

    typedef char FieldType;
    // Bit Ranges within FieldType defined by 1st bit and size:
    typedef bit::Range<FieldType,0,                    1> StateField;
    typedef bit::Range<FieldType,StateField::end,      2> EstablishField;
    typedef bit::Range<FieldType,EstablishField::end,  1> TransactField;
    typedef bit::Range<FieldType,TransactField::end,   2> TransactByField;
    typedef bit::Range<FieldType,TransactByField::end, 2> TransactDetailField;
    // enlarge FieldType if more bit's needed. It's not yet
    // checked by the compiler.
  public:

    // do we manage to hide them

    enum StateValue
      {
        UNINSTALLED = 0,
        INSTALLED   = StateField::minval
      };
    enum EstablishValue
      {
        UNDETERMINED = 0,
        UNNEEDED     = EstablishField::minval,
        SATISFIED,
        INCOMPLETE
      };
    enum TransactValue
      {
        KEEP_STATE = 0,
        TRANSACT = TransactField::minval		// change state installed <-> uninstalled
      };
    enum TransactByValue
      {
        SOLVER = 0,
        APPL_LOW  = TransactByField::minval,
        APPL_HIGH,
        USER
      };

    enum InstallDetailValue
      {
        EXPLICIT_INSTALL = 0,
        SOFT_REQUIRES = TransactDetailField::minval
      };
    enum RemoveDetailValue
      {
        EXPLICIT_REMOVE = 0,
        DUE_TO_OBSOLETE = TransactDetailField::minval,
        DUE_TO_UNLINK
      };

  public:

    /** Default ctor. */
    ResStatus();

    /** Ctor setting the initial . */
    ResStatus( bool isInstalled_r );

    /** Dtor. */
    ~ResStatus();

  public:

    bool isInstalled() const
    { return fieldValueIs<StateField>( INSTALLED ); }

    bool isToBeInstalled() const
    { return isUninstalled() && transacts(); }

    bool isUninstalled() const
    { return fieldValueIs<StateField>( UNINSTALLED ); }

    bool isToBeUninstalled() const
    { return isInstalled() && transacts(); }

    bool isUnneeded() const
    { return fieldValueIs<EstablishField>( UNNEEDED ); }

    bool isSatisfied() const
    { return fieldValueIs<EstablishField>( SATISFIED ); }

    bool isIncomplete() const
    { return fieldValueIs<EstablishField>( INCOMPLETE ); }

    bool transacts() const
    { return fieldValueIs<TransactField>( TRANSACT ); }

    bool isToBeUninstalledDueToObsolete () const
    { return isToBeUninstalled() && fieldValueIs<TransactDetailField>( DUE_TO_OBSOLETE ); }

    bool isToBeUninstalledDueToUnlink() const
    { return isToBeUninstalled() && fieldValueIs<TransactDetailField>( DUE_TO_UNLINK); }

    bool isToBeInstalledSoft () const
    { return isToBeInstalled() && fieldValueIs<TransactDetailField> (SOFT_REQUIRES); }

  public:

    bool setTransacts( bool val_r )
    {
      fieldValueAssign<TransactField>( val_r ? TRANSACT : KEEP_STATE );
      return true;
    }

    bool setToBeInstalled ( )
    {
      if (isInstalled()) return false;
      return setTransacts (true);
    }

    bool setToBeInstalledSoft ( )
    {
      if (!setToBeInstalled()) return false;
      fieldValueAssign<TransactDetailField>(SOFT_REQUIRES);
      return true;
    }

    bool setToBeUninstalled ( )
    {
      if (isUninstalled()) return false;
      return setTransacts (true);
    }

    bool setToBeUninstalledDueToUnlink ( )
    {
      if (!setToBeUninstalled()) return false;
      fieldValueAssign<TransactDetailField>(DUE_TO_UNLINK);
      return true;
    }

    bool setToBeUninstalledDueToObsolete ( )
    {
      if (!setToBeUninstalled()) return false;
      fieldValueAssign<TransactDetailField>(DUE_TO_OBSOLETE);
      return true;
    }

    // *** These are only for the Resolver ***

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

    // get/set functions, returnig \c true if requested status change
    // was successfull (i.e. leading to the desired transaction).
    // If a lower level (e.g.SOLVER) wants to transact, but it's
    // already set by a higher level, \c true should be returned.
    // Removing a higher levels transaction bit should fail.
  private:

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

  private:
    bit::BitField<FieldType> _bitfield;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ResStatus Stream output */
  std::ostream & operator<<( std::ostream & str, const ResStatus & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESSTATUS_H
