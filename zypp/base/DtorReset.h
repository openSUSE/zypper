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
    template<class _Var>
      DtorReset( _Var & var_r )
      : _pimpl( new Impl<_Var,_Var>( var_r, var_r ) )
      {}
    template<class _Var, class _Val>
      DtorReset( _Var & var_r, const _Val & val_r )
      : _pimpl( new Impl<_Var,_Val>( var_r, val_r ) )
      {}

  private:
    /** Requires _Val being copy constructible, and assignment
     * <tt>_Var = _Val</tt> defined. */
    template<class _Var, class _Val>
      struct Impl
      {
        Impl( _Var & var_r, const _Val & val_r )
        : _var( var_r )
        , _val( val_r )
        {}
        ~Impl()
        { _var = _val; }
        _Var & _var;
        _Val   _val;
      };
    shared_ptr<void> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_DTORRESET_H
