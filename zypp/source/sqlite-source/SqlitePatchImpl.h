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

      /** Is the patch installation interactive? (does it need user input?) */
      virtual bool interactive() const;

      /** The list of all atoms building the patch */
      virtual AtomList all_atoms() const
      { return AtomList(); }
      /** The list of those atoms which have not been installed */
      virtual AtomList not_installed_atoms() const
      { return AtomList(); }

// TODO check necessarity of functions below
      virtual void mark_atoms_to_freshen(bool freshen)
      { return; }
      virtual bool any_atom_selected() const
      { return false; }

protected:
	Source_Ref _source;
	ZmdId _zmdid;
	std::string _id;
	Date _timestamp;
	std::string _category;
	bool _reboot_needed;
	bool _affects_pkg_manager;
	bool _interactive;
	ByteCount _size_installed;
	ByteCount _size_archive;


 };
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZMD_BACKEND_DBSOURCE_DBPATCHIMPL_H
