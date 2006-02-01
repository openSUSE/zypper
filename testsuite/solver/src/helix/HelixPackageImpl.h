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
#ifndef ZYPP_SOLVER_TEMPORARY_HELIXPACKAGEIMPL_H
#define ZYPP_SOLVER_TEMPORARY_HELIXPACKAGEIMPL_H

#include "zypp/detail/PackageImpl.h"
#include "HelixParser.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : HelixPackageImpl
//
/** Class representing a package
*/
class HelixPackageImpl : public detail::PackageImplIf
{
public:

	class HelixParser;
	/** Default ctor
	*/
	HelixPackageImpl( Source_Ref source_r, const zypp::HelixParser & data );

	/** Package summary */
	virtual TranslatedText summary() const;
	/** Package description */
	virtual TranslatedText description() const;
	virtual ByteCount size() const;
	/** */
	virtual PackageGroup group() const;
	/** */
	virtual ByteCount archivesize() const;
	/** */
	virtual bool installOnly() const;
	/** */
	virtual Source_Ref source() const;

protected:
	Source_Ref _source;
	TranslatedText _summary;
	TranslatedText _description;
	PackageGroup _group;
	bool _install_only;

	ByteCount _size_installed;
	ByteCount _size_archive;


 };
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOLVER_TEMPORARY_HELIXPACKAGEIMPL_H
