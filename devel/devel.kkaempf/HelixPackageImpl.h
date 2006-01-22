/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/solver/temporary/HelixPackageImpl.h
 *
*/
#ifndef ZYPP_SOLVER_TEMPORARY_RCPACKAGEIMPL_H
#define ZYPP_SOLVER_TEMPORARY_RCPACKAGEIMPL_H

#include "zypp/detail/PackageImpl.h"
#include "HelixParser.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////
    namespace detail
    { //////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //        CLASS NAME : HelixPackageImpl
      //
      /** Class representing a package
      */
      class HelixPackageImpl : public zypp::detail::PackageImplIf
      {
      public:

	class HelixParser;
	/** Default ctor
	*/
	HelixPackageImpl(
	  const HelixParser & data
	);
#if 0
	/** Package summary */
	virtual Label summary() const;
	/** Package description */
	virtual Text description() const;
	virtual ByteCount size() const;
	/** */
	virtual PackageGroup group() const;
	/** */
	virtual ByteCount archivesize() const;
	/** */

      protected:
	Label _summary;
	Text _description;
	PackageGroup _group;

	ByteCount _size_installed;
	ByteCount _size_archive;
#endif

       };
      ///////////////////////////////////////////////////////////////////
    } // namespace detail
    /////////////////////////////////////////////////////////////////
  } // namespace solver
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOLVER_TEMPORARY_HELIXPACKAGEIMPL_H
