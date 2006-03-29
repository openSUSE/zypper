/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/testsuite/solver/HelixLanguageImpl.h
 *
*/
#ifndef ZYPP_HELIXLANGUAGEIMPL_H
#define ZYPP_HELIXLANGUAGEIMPL_H

#include "zypp/Language.h"
#include "HelixParser.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : HelixLanguageImpl
//
/** Class representing a package
*/
class HelixLanguageImpl : public detail::LanguageImplIf
{
public:

	class HelixParser;
	/** Default ctor
	*/
	HelixLanguageImpl( Source_Ref source_r, const zypp::HelixParser & data );

protected:

 };
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_HELIXLANGUAGEIMPL_H
