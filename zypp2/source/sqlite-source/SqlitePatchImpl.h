/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbsource/SqlitePatchImpl.h
 *
*/
#ifndef ZMD_BACKEND_DBSOURCE_DBPATCHIMPL_H
#define ZMD_BACKEND_DBSOURCE_DBPATCHIMPL_H

#include "zypp/detail/PatchImpl.h"
#include "zypp/Source.h"
#include <sqlite3.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : SqlitePatchImpl
//
/** Class representing a package
*/
class SqlitePatchImpl : public detail::PatchImplIf
{
public:

	/** Default ctor
	*/
	SqlitePatchImpl( Source_Ref source_r );
	void readHandle( sqlite_int64 id, sqlite3_stmt *handle );

	/** */
	virtual Source_Ref source() const;
        /** */
	virtual ZmdId zmdid() const;

      /** Patch ID */
      virtual std::string id() const;
      /** Patch time stamp */
      virtual Date timestamp() const;
      /** Patch category (recommended, security,...) */
      virtual std::string category() const;
      /** Does the system need to reboot to finish the update process? */
      virtual bool reboot_needed() const;
      /** Does the patch affect the package manager itself? */
      virtual bool affects_pkg_manager() const;
      /** */
      virtual ByteCount size() const;
      /** The list of all atoms building the patch */
      virtual AtomList all_atoms() const
      { return AtomList(); }

protected:
	Source_Ref _source;
	ZmdId _zmdid;
	std::string _id;
	Date _timestamp;
	std::string _category;
	bool _reboot_needed;
	bool _affects_pkg_manager;
        /** \todo remove it as it is not needed, PatchImplIf::interactive()
         * should be sufficient, usg. no need to overload it. */
	bool _interactive;
	ByteCount _size_installed;
	ByteCount _size_archive;


 };
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZMD_BACKEND_DBSOURCE_DBPATCHIMPL_H
