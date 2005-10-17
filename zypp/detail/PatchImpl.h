/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/PatchImpl.h
 *
*/
#ifndef ZYPP_DETAIL_PATCHIMPL_H
#define ZYPP_DETAIL_PATCHIMPL_H

#include <map>
#include <list>
#include <string>

#include "zypp/detail/ResolvableImpl.h"
#include "zypp/Resolvable.h"
#include "zypp/Patch.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

//    typedef std::list<Resolvable> atom_list;
    #define atom_list std::list<ResolvablePtr>

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PatchImpl
    //
    /** */
    class PatchImpl : public ResolvableImpl
    {
    public:
      /** Default ctor */
      PatchImpl( const ResName & name_r,
		 const Edition & edition_r,
		 const Arch & arch_r );
      /** Dtor */
      ~PatchImpl();

    public:
      std::string _category;
      std::map<std::string,std::string> _summary;
      std::map<std::string,std::string> _description;
      int _timestamp;
      std::string _patch_id;
      bool _reboot_needed;
      bool _package_manager;

      bool interactive ();
      bool any_atom_selected ();
      void mark_atoms_to_freshen (bool freshen);
    protected:
      atom_list not_installed_atoms ();
      atom_list _atoms;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PATCHIMPL_H
