/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Target.h
 *
*/
#ifndef ZYPP_TARGET_H
#define ZYPP_TARGET_H

#include <iosfwd>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/Deprecated.h"

#include "zypp/ResStore.h"
#include "zypp/Pathname.h"
#include "zypp/ResPool.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  namespace target
  {
    class TargetImpl;
    namespace rpm {
      class RpmDb;
    }
  }
  namespace zypp_detail
  {
    class ZYppImpl;
  }

  DEFINE_PTR_TYPE(Target);

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Target
  //
  /**
  */
  class Target : public base::ReferenceCounted, public base::NonCopyable
  {
  public:
    typedef target::TargetImpl  Impl;
    typedef intrusive_ptr<Impl> Impl_Ptr;
    typedef std::list<PoolItem_Ref> PoolItemList;

  public:

    /** All resolvables provided by the target. */
    const ResStore & resolvables();

    /** Null implementation */
    static Target_Ptr nullimpl();

    /** Refference to the RPM database */
    target::rpm::RpmDb & rpmDb();

    /** If the package is installed and provides the file
     Needed to evaluate split provides during Resolver::Upgrade() */
    bool providesFile (const std::string & name_str, const std::string & path_str) const;

    ResObject::constPtr whoOwnsFile (const std::string & path_str) const;

    /** JUST FOR TESTSUITE */
    /** Sort according to prereqs and media numbers
     * \todo provide it as standalone algorithm
    */
    void getResolvablesToInsDel ( const ResPool pool_r,
                                  PoolItemList & dellist_r,
                                  PoolItemList & instlist_r,
                                  PoolItemList & srclist_r ) const;

#ifndef STORAGE_DISABLED
    /** enables the storage target */
    bool isStorageEnabled() const;
    void enableStorage(const Pathname &root_r);
#endif

    /** Set the log file for target */
    bool setInstallationLogfile(const Pathname & path_r);

    /** Return the root set for this target */
    Pathname root() const;

    /** return the last modification date of the target */
    Date timestamp() const;
  public:
    /** Ctor */
    explicit
    Target( const Pathname & root = "/" );
    /** Ctor */
    explicit
    Target( const Impl_Ptr & impl_r );

  private:
    friend std::ostream & operator<<( std::ostream & str, const Target & obj );
    /** Stream output. */
    std::ostream & dumpOn( std::ostream & str ) const;

  private:
    /** Direct access to Impl. */
    friend class zypp_detail::ZYppImpl;

    /** Pointer to implementation */
    RW_pointer<Impl,rw_pointer::Intrusive<Impl> > _pimpl;

    static Target_Ptr _nullimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Target Stream output. */
  inline std::ostream & operator<<( std::ostream & str, const Target & obj )
  { return obj.dumpOn( str ); }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_H
