/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ZYpp.h
 *
*/
#ifndef ZYPP_ZYPP_H
#define ZYPP_ZYPP_H

#include <iosfwd>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/Target.h"
#include "zypp/Resolver.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace zypp_detail
  {
    class ZYppImpl;
  }

  class ZYppFactory;
  class ResPool;
  class ResPoolProxy;
  class SourceFeed_Ref;
  class ResStore;

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ZYpp
  //
  /** */
  class ZYpp : public base::ReferenceCounted, private base::NonCopyable
  {
  public:

    typedef intrusive_ptr<ZYpp>       Ptr;
    typedef intrusive_ptr<const ZYpp> constPtr;

  public:

    /** Pool of ResStatus for individual ResObjetcs. */
    ResPool pool() const;

    /** Pool of ui::Selectable.
     * Based on the ResPool, ui::Selectable groups ResObjetcs of
     * same kind and name.
    */
    ResPoolProxy poolProxy() const;

    /**  */
    SourceFeed_Ref sourceFeed() const;

    void addResolvables (const ResStore& store, bool installed = false);

    void removeResolvables (const ResStore& store);

    /**
     * \throws Exception
     */
    Target_Ptr target() const;

    /**
     * \throws Exception
     * if commit_only == true, just init, don't populate store or pool
     */
    void initTarget(const Pathname & root, bool commit_only = false);

    /**
     * \throws Exception
     */
    void finishTarget();

    /**
     *
     */
    Resolver_Ptr resolver() const;

  protected:
    /** Dtor */
    virtual ~ZYpp();
    /** Stream output */
    virtual std::ostream & dumpOn( std::ostream & str ) const;
  private:
    /** Factory */
    friend class ZYppFactory;
    /** */
    typedef zypp_detail::ZYppImpl Impl;
    typedef shared_ptr<Impl>      Impl_Ptr;
    /** Factory ctor */
    explicit
    ZYpp( const Impl_Ptr & impl_r );
  private:
    /** Pointer to implementation */
    RW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ZYPP_H
