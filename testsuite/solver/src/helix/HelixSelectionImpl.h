/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/solver/temporary/HelixSelectionImpl.h
 *
*/
#ifndef ZYPP_SOLVER_TEMPORARY_HELIXSELECTIONIMPL_H
#define ZYPP_SOLVER_TEMPORARY_HELIXSELECTIONIMPL_H

#include "zypp/detail/SelectionImpl.h"
#include "HelixParser.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : HelixSelectionImpl
//
/** Class representing a package
*/
class HelixSelectionImpl : public detail::SelectionImplIf
{
public:

	class HelixParser;
	/** Default ctor
	*/
	HelixSelectionImpl( Source_Ref source_r, const zypp::HelixParser & data );

	/** Selection summary */
	virtual TranslatedText summary() const;
	/** Selection description */
	virtual TranslatedText description() const;

	/** */
	virtual Source_Ref source() const;
protected:
	Source_Ref _source;
	TranslatedText _summary;
	TranslatedText _description;

 };
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOLVER_TEMPORARY_HELIXSELECTIONIMPL_H
