/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/solver/temporary/HelixProductImpl.h
 *
*/
#ifndef ZYPP_SOLVER_TEMPORARY_HELIXPRODUCTIMPL_H
#define ZYPP_SOLVER_TEMPORARY_HELIXPRODUCTIMPL_H

#include "zypp/detail/ProductImpl.h"
#include "HelixParser.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : HelixProductImpl
//
/** Class representing a package
*/
class HelixProductImpl : public detail::ProductImplIf
{
public:

	class HelixParser;
	/** Default ctor
	*/
	HelixProductImpl( Source_Ref source_r, const zypp::HelixParser & data );

	/** Product summary */
	virtual TranslatedText summary() const;
	/** Product description */
	virtual TranslatedText description() const;
	virtual ByteCount size() const;
	/** */
	virtual PackageGroup group() const;
	/** */
	virtual ByteCount archivesize() const;
	/** */
	virtual bool installOnly() const;
	/** */

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
