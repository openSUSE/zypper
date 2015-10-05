/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/DtorReset.h
 *
*/
#ifndef ZYPP_BASE_DTORRESET_H
#define ZYPP_BASE_DTORRESET_H

#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : DtorReset
  //
  /** Assign a vaiable a certain value when going out of scope.
   * Use it e.g. to reset/cleanup in presence of exceptions.
   * \code
   * struct Foo
   * {
   *   void consume()
   *   {
   *     DtorReset x(_inConsume,false);
   *     _inConsume = true;
   *     MIL << _inConsume << endl;
   *   };
   *
   *   DefaultIntegral<bool,false> _inConsume;
   * };
   *
   * Foo f;
   * MIL << f._inConsume << endl; // 0
   * f.consume();                 // 1
   * MIL << f._inConsume << endl; // 0
   * \endcode
   * \ingroup g_RAII
   * \todo Check if using call_traits enables 'DtorReset(std::string,"value")',
   * as this currently would require assignment of 'char[]'.
   */
  class DtorReset
  {
  public:
    template<class TVar>
      DtorReset( TVar & var_r )
      : _pimpl( new Impl<TVar,TVar>( var_r, var_r ) )
      {}
    template<class TVar, class TVal>
      DtorReset( TVar & var_r, const TVal & val_r )
      : _pimpl( new Impl<TVar,TVal>( var_r, val_r ) )
      {}

  private:
    /** Requires TVal being copy constructible, and assignment
     * <tt>TVar = TVal</tt> defined. */
    template<class TVar, class TVal>
      struct Impl
      {
        Impl( TVar & var_r, const TVal & val_r )
        : _var( var_r )
        , _val( val_r )
        {}
        ~Impl()
        { _var = _val; }
        TVar & _var;
        TVal   _val;
      };
    shared_ptr<void> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_DTORRESET_H
