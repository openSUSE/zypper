/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/solver/temporary/HelixAtomImpl.h
 *
*/
#ifndef ZYPP_SOLVER_TEMPORARY_HELIXATOMIMPL_H
#define ZYPP_SOLVER_TEMPORARY_HELIXATOMIMPL_H

#include "zypp/detail/AtomImpl.h"
#include "HelixParser.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : HelixAtomImpl
//
/** Class representing a package
*/
class HelixAtomImpl : public detail::AtomImplIf
{
public:

	class HelixParser;
	/** Default ctor
	*/
	HelixAtomImpl( Source_Ref source_r, const zypp::HelixParser & data );

	/** */
	virtual Source_Ref source() const;

protected:
	Source_Ref _source;


 };
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOLVER_TEMPORARY_HELIXATOMIMPL_H
