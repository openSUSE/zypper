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
  //
  //	CLASS NAME : Dependencies
  //
  /**
   * \invariant No NULL _pimpl.
  */
  class Dependencies
  {
    friend std::ostream & operator<<( std::ostream & str, const Dependencies & obj );
  public:

    /** Implementation */
    struct Impl;

  public:
    /** Default ctor */
    Dependencies();
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
    const CapSet & freshens() const;

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
    /**  */
    void setFreshens( const CapSet & val_r );

  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Dependencies Stream output */
  extern std::ostream & operator<<( std::ostream & str, const Dependencies & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DEPENDENCIES_H
