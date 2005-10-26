/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Dependencies.h
 *
*/
#ifndef ZYPP_DEPENDENCIES_H
#define ZYPP_DEPENDENCIES_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

#include "zypp/CapSetFwd.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////
    DEFINE_PTR_TYPE(DependenciesImpl)
    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Dependencies
  //
  /** */
  class Dependencies
  {
  public:
    /** Default ctor */
    Dependencies();
    /** Factory ctor */
    explicit
    Dependencies( detail::DependenciesImplPtr impl_r );
    /** Dtor */
    ~Dependencies();

  public:
    /**  */
    const CapSet & provides() const;
    /**  */
    const CapSet & prerequires() const;
    /**  */
    const CapSet & requires() const;
    /**  */
    const CapSet & conflicts() const;
    /**  */
    const CapSet & obsoletes() const;
    /**  */
    const CapSet & recommends() const;
    /**  */
    const CapSet & suggests() const;

    /**  */
    void setProvides( const CapSet & val_r );
    /**  */
    void setPrerequires( const CapSet & val_r );
    /**  */
    void setRequires( const CapSet & val_r );
    /**  */
    void setConflicts( const CapSet & val_r );
    /**  */
    void setObsoletes( const CapSet & val_r );
    /**  */
    void setRecommends( const CapSet & val_r );
    /**  */
    void setSuggests( const CapSet & val_r );

  private:
    /** Pointer to implementation */
    detail::DependenciesImplPtr _pimpl;
  public:
    /** Avoid a bunch of friend decl. */
    detail::constDependenciesImplPtr sayFriend() const;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Dependencies Stream output */
  extern std::ostream & operator<<( std::ostream & str, const Dependencies & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DEPENDENCIES_H
