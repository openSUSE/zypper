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
      virtual Date timestamp() const ;
      /** Patch category (recommended, security,...) */
      virtual std::string category() const ;
      /** Does the system need to reboot to finish the update process? */
      virtual bool reboot_needed() const ;
      /** Does the patch affect the package manager itself? */
      virtual bool affects_pkg_manager() const ;
      /** */
      virtual ByteCount size() const;
      /** The list of all atoms building the patch */
      virtual AtomList all_atoms() const;
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
