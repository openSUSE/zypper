/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/TargetImpl.h
 *
*/
#ifndef ZYPP_TARGET_TARGETIMPL_H
#define ZYPP_TARGET_TARGETIMPL_H

#include <iosfwd>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/ResStore.h"

#include "zypp/Pathname.h"
#include "zypp/media/MediaAccess.h"
#include "zypp/Target.h"
#include "zypp/target/rpm/RpmDb.h"
#include "zypp/target/store/PersistentStorage.h"
#include "zypp/solver/detail/Types.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(TargetImpl);

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : TargetImpl
    //
    /** Base class for concrete Target implementations.
     *
     * Constructed by \ref TargetFactory. Public access via \ref Target
     * interface.
    */
    class TargetImpl : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const TargetImpl & obj );

    private:
      /** Sort according to prereqs and media numbers */
      void getResolvablesToInsDel ( const ResPool pool_r,
				    PoolItemList & dellist_r,
				    PoolItemList & instlist_r,
				    PoolItemList & srclist_r );


    public:
      /** Ctor. */
      TargetImpl(const Pathname & root_r = "/");
      /** Dtor. */
      virtual ~TargetImpl();

      /** Null implementation */
      static TargetImpl_Ptr nullimpl();

    public:

      /** All resolvables in the target. */
      const ResStore & resolvables();

      /** Commit changes in the pool
	  media = 0 means any/all medias
	  media > 0 means limit commits to this media */
      void commit( ResPool pool_r, unsigned int medianr, PoolItemList & errors_r, PoolItemList & remaining_r, PoolItemList & srcremaining_r );

      /** enables the storage target */
      bool isStorageEnabled() const;
      void enableStorage(const Pathname &root_r);

      /** Commit ordered changes
	  return uncommitted ones (due to error) */
      PoolItemList commit( const PoolItemList & items_r );

      /** Overload to realize stream output. */
      virtual std::ostream & dumpOn( std::ostream & str ) const
      { return str << "TargetImpl"; }

      /** The RPM database */
      rpm::RpmDb & rpm();

      /** If the package is installed and provides the file
	  Needed to evaluate split provides during Resolver::Upgrade() */
      bool providesFile (const std::string & path_str, const std::string & name_str) const;

      /** Return the resolvable which provides path_str (rpm -qf)
	  return NULL if no resolvable provides this file  */
      ResObject::constPtr whoOwnsFile (const std::string & path_str) const;

    protected:
      /** All resolvables provided by the target. */
      ResStore _store;
      /** Path to the target */
      Pathname _root;
      /** RPM database */
      rpm::RpmDb _rpm;
#ifndef STORAGE_DISABLED
      zypp::storage::PersistentStorage _storage;
#endif
    private:
      /** Null implementation */
      static TargetImpl_Ptr _nullimpl;
      
      /** wrapper with callback around getPlainRpm */
      Pathname getRpmFile(Package::constPtr package);
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates TargetImpl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const TargetImpl & obj )
    { return obj.dumpOn( str ); }

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_TARGETIMPL_H
