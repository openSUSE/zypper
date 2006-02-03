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

#include "zypp/base/PtrTypes.h"

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

  public:

    /** All resolvables provided by the target. */
    const ResStore & resolvables();
    /** Null implementation */
    static Target_Ptr nullimpl();
    /** Refference to the RPM database */
    target::rpm::RpmDb & rpmDb();
    /** Commit changes in the pool */
    void commit(ResPool pool_r);

      /** If the package is installed and provides the file
	  Needed to evaluate split provides during Resolver::Upgrade() */
      bool providesFile (const std::string & name_str, const std::string & path_str) const;

      ResObject::constPtr whoOwnsFile (const std::string & path_str) const;

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
    /** Pointer to implementation */
    RW_pointer<Impl,rw_pointer::Intrusive<Impl> > _pimpl;

    static Target_Ptr _nullimpl;

  public:
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Target Stream output. */
  inline std::ostream & operator<<( std::ostream & str, const Target & obj )
  { return obj.dumpOn( str ); }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_H
