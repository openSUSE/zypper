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
   * \li \c StateField Whether the resolvable is installed
   *        or uninstalled (available).
   * \li \c FreshenField Freshen status computed by the solver.
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
    typedef bit::Range<FieldType,StateField::end,      2> FreshenField;
    typedef bit::Range<FieldType,FreshenField::end,    1> TransactField;
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
    enum FreshenValue
      {
        UNDETERMINED = 0,
        NO_TRIGGER   = FreshenField::minval,
        TRIGGER_OK,
        TRIGGER_FAILED
      };
    enum TransactValue
      {
        KEEP_STATE = 0,
        TRANSACT = TransactField::minval
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

    /** Ctor seting the initail . */
    ResStatus( bool isInstalled_r );

    /** Dtor. */
    ~ResStatus();

  public:

    bool isInstalled() const
    { return fieldValueIs<StateField>( INSTALLED ); }

    bool transacts()   const
    { return fieldValueIs<TransactField>( TRANSACT ); }

  public:

    bool setTransacts( bool val_r )
    {
      fieldValueAssign<TransactField>( val_r ? TRANSACT : KEEP_STATE );
      return true;
    }


    // get/set functions, returnig \c true if requested status change
    // was successfull (i.e. leading to the desired transaction).
    // If a lower level (e.g.SOLVER) wants to transact, but it's
    // already set by a higher level, \c true should be returned.
    // Removing a higher levels transaction bit should fail.
  private:

    /** Return wheter the corresponding Field has value \a val_r.
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
