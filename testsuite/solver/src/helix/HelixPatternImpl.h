/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/solver/temporary/HelixPatternImpl.h
 *
*/
#ifndef ZYPP_SOLVER_TEMPORARY_HELIXPATTERNIMPL_H
#define ZYPP_SOLVER_TEMPORARY_HELIXPATTERNIMPL_H

#include "zypp/detail/PatternImpl.h"
#include "HelixParser.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : HelixPatternImpl
//
/** Class representing a package
*/
class HelixPatternImpl : public detail::PatternImplIf
{
public:

	class HelixParser;
	/** Default ctor
	*/
	HelixPatternImpl( Source_Ref source_r, const zypp::HelixParser & data );

	/** Pattern summary */
	virtual Label summary() const;
	/** Pattern description */
	virtual Text description() const;
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
	Label _summary;
	Text _description;
	PackageGroup _group;
	bool _install_only;

	ByteCount _size_installed;
	ByteCount _size_archive;


 };
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOLVER_TEMPORARY_HELIXPACKAGEIMPL_H
