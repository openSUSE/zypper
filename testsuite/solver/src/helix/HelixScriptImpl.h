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
#ifndef ZYPP_SOLVER_TEMPORARY_HELIXSCRIPTIMPL_H
#define ZYPP_SOLVER_TEMPORARY_HELIXSCRIPTIMPL_H

#include "zypp/detail/ScriptImpl.h"
#include "HelixParser.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : HelixScriptImpl
//
/** Class representing a script
*/
class HelixScriptImpl : public detail::ScriptImplIf
{
public:

	class HelixParser;
	/** Default ctor
	*/
	HelixScriptImpl( Source_Ref source_r, const zypp::HelixParser & data );

      /** Get the script to perform the change */
      virtual Pathname do_script() const;
      /** Get the script to undo the change */
      virtual Pathname undo_script() const;
      /** Check whether script to undo the change is available */
      virtual bool undo_available() const;
      /** */
      virtual ByteCount size() const;

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
