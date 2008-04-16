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
    typedef std::list<PoolItem> PoolItemList;

  public:

    /**
     * builds or refreshes the target cache
     */
    void buildCache();

    /**
     * load resolvables into the pool
     */
    void load();

    /**
     * unload target resolvables from the
     * pool
     */
    void unload();

    /** \deprecated NOOP
    */
    ZYPP_DEPRECATED void reset() {}

    /** Null implementation */
    static Target_Ptr nullimpl();

    /** Refference to the RPM database */
    target::rpm::RpmDb & rpmDb();

    /** If the package is installed and provides the file
     Needed to evaluate split provides during Resolver::Upgrade() */
    bool providesFile (const std::string & name_str, const std::string & path_str) const;

    /** Return name of package owning \a path_str
     * or empty string if no installed package owns \a path_str.
     **/
    std::string whoOwnsFile (const std::string & path_str) const;

    /** Set the log file for target */
    bool setInstallationLogfile(const Pathname & path_r);

    /** Return the root set for this target */
    Pathname root() const;

    /** return the last modification date of the target */
    Date timestamp() const;

  public:
    /** Ctor. If \c doRebuild_r is \c true, an already existing
     * database is rebuilt (rpm --rebuilddb ).
    */
    explicit
    Target( const Pathname & root = "/", bool doRebuild_r = false );
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
