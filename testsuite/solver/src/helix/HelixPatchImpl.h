/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/solver/temporary/HelixPatchImpl.h
 *
*/
#ifndef ZYPP_SOLVER_TEMPORARY_HELIXPATCHIMPL_H
#define ZYPP_SOLVER_TEMPORARY_HELIXPATCHIMPL_H

#include "zypp/detail/PatchImpl.h"
#include "HelixParser.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : HelixPatchImpl
//
/** Class representing a package
*/
class HelixPatchImpl : public detail::PatchImplIf
{
public:

	class HelixParser;
	/** Default ctor
	*/
	HelixPatchImpl( Source_Ref source_r, const zypp::HelixParser & data );


      /** Patch ID */
      virtual std::string id() const ;
      /** Patch time stamp */
      virtual unsigned int timestamp() const ;
      /** Patch category (recommended, security,...) */
      virtual std::string category() const ;
      /** Does the system need to reboot to finish the update process? */
      virtual bool reboot_needed() const ;
      /** Does the patch affect the package manager itself? */
      virtual bool affects_pkg_manager() const ;
      /** */
      virtual ByteCount size() const;

      /** Is the patch installation interactive? (does it need user input?) */
      virtual bool interactive() const;
      /** The list of all atoms building the patch */
      virtual AtomList all_atoms() const;
      /** The list of those atoms which have not been installed */
      virtual AtomList not_installed_atoms() const;

// TODO check necessarity of functions below
      virtual void mark_atoms_to_freshen(bool freshen) ;
      virtual bool any_atom_selected() const;

	/** */
	virtual Source_Ref source() const;

protected:
	Source_Ref _source;
	ByteCount _size_installed;
 };
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOLVER_TEMPORARY_HELIXPACKAGEIMPL_H
