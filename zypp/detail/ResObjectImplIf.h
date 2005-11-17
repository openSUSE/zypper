/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/ResObjectImplIf.h
 *
*/
#ifndef ZYPP_DETAIL_RESOBJECTIMPLIF_H
#define ZYPP_DETAIL_RESOBJECTIMPLIF_H

#include <list>
#include <string>

#include "zypp/detail/ResObjectFactory.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Resolvable;

  typedef std::string            line;
  typedef std::list<std::string> text;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ResObjectImplIf
    //
    /** Abstact ResObject implementation interface.
    */
    class ResObjectImplIf
    {
    public:
      /** */
      const Resolvable *const self() const
      { return _backRef; }
      /** */
      Resolvable *const self()
      { return _backRef; }
      /** */
      virtual line summary() const
      { return line(); }
      /** */
      virtual text description() const
      { return text(); }

    public:
      /** Ctor */
      ResObjectImplIf()
      : _backRef( 0 )
      {}
      /** Dtor. Makes this an abstract class. */
      virtual ~ResObjectImplIf() = 0;

    private:
      /** */
      template<class _Res>
        friend class _resobjectfactory_detail::ResImplConnect;
      /** */
      Resolvable * _backRef;
    };
    /////////////////////////////////////////////////////////////////

    /* Implementation of pure virtual dtor is required! */
    inline ResObjectImplIf::~ResObjectImplIf()
    {}

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_RESOBJECTIMPLIF_H
