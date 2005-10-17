/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/ScriptImpl.h
 *
*/
#ifndef ZYPP_DETAIL_SCRIPTIMPL_H
#define ZYPP_DETAIL_SCRIPTIMPL_H

#include <iosfwd>

#include "zypp/detail/ResolvableImpl.h"
#include "zypp/Resolvable.h"
#include "zypp/Script.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ScriptImpl
    //
    /** */
    class ScriptImpl : public ResolvableImpl
    {
    public:
      /** Default ctor */
      ScriptImpl( const ResName & name_r,
		  const Edition & edition_r,
		  const Arch & arch_r );
      /** Dtor */
      ~ScriptImpl();

    public:
      std::string do_script () const;
      std::string undo_script () const;
      bool undo_available () const;
    protected:
      std::string _do_script;
      std::string _undo_script;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SCRIPTIMPL_H
