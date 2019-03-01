/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_GUARDIANS_H
#define ZYPPER_GUARDIANS_H

#include <zypp/base/PtrTypes.h>

///////////////////////////////////////////////////////////////////
/// \class Guardians<TreasureT>
/// \brief Simple static guardians protecting a TreasureT
///
/// Basically \c shared_ptr/weak_ptr<TreasureT>.
/// \code
///  struct SigExitTreasureT {
///    friend std::ostream & operator<<( std::ostream & str, SigExitTreasureT obj )
///    { return str << "SigExitTreasure"; }
///  };
///  typedef Guardians<SigExitTreasureT> SigExitGuardians;
///  typedef SigExitGuardians::Guard     SigExitGuard;
///
///  static SigExitGuard sigExitGuard()
///  { return SigExitGuardians::guard(); }
/// \endcode
template <typename TreasureT>
struct Guardians
{
  /** The Guards type. */
  typedef zypp::shared_ptr<TreasureT> Guard;

  /** Get a Guard protecting the treasure. */
  static Guard guard() {
    Guard ret { hideout().lock() };
    if ( !ret )
    {
      ret.reset( new TreasureT );
      hideout() = ret;
    }
    return ret;
  }

  /** Return whether the treasure is unguarded. */
  static bool expired()
  { return hideout().expired(); }

  friend std::ostream & operator<<( std::ostream & str, Guardians obj )
  { return str << TreasureT() << ( expired() ? "-Void" : "-Guarded" ); }

private:
  static zypp::weak_ptr<TreasureT> & hideout()
  { static zypp::weak_ptr<TreasureT> _hideout; return _hideout; }
};

#endif // ZYPPER_GUARDIANS_H
