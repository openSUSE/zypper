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

#include<list>
#include<string>

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
      const Resolvable * self() const
      { return _backRef; }
      /** */
      Resolvable * self()
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
      /** Dtor */
      virtual ~ResObjectImplIf() = 0;

    private:
      /** */
      template<class _Res>
        friend class ResImplConnect;
      /** */
      Resolvable * _backRef;
    };
    /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_RESOBJECTIMPLIF_H
