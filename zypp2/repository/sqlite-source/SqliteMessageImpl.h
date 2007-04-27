/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbsource/SqliteMessageImpl.h
 *
*/
#ifndef ZMD_BACKEND_DBSOURCE_DBMESSAGEIMPL_H
#define ZMD_BACKEND_DBSOURCE_DBMESSAGEIMPL_H

#include "zypp/detail/MessageImpl.h"
#include "zypp/Source.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : SqliteMessageImpl
//
/** Class representing a package
*/
class SqliteMessageImpl : public detail::MessageImplIf
{
public:

	/** Default ctor
	*/
	SqliteMessageImpl( Source_Ref source_r, TranslatedText text, ZmdId zmdid );

	/** */
	virtual Source_Ref source() const;
	virtual TranslatedText text() const;
	virtual ByteCount size() const;
        /** */
	virtual ZmdId zmdid() const;

protected:
	Source_Ref _source;
	TranslatedText _text;
	ZmdId _zmdid;

 };
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZMD_BACKEND_DBSOURCE_DBMESSAGEIMPL_H
