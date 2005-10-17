/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Edition.h
 *
*/
#ifndef ZYPP_EDITION_H
#define ZYPP_EDITION_H

#include <iosfwd>
#include <string>

#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Edition
  //
  /** */
  class Edition
  {
  public:
    typedef unsigned epoch_t;
  public:
    /** Default ctor */
    Edition();
    /** */
    Edition( const std::string & version_r,
             const std::string & release_r,
             epoch_t epoch = 0 );
    /** Dtor */
    ~Edition();
  public:
    /** */
    epoch_t epoch() const;
    /** */
    const std::string & version() const;
    /** */
    const std::string & release() const;
  private:
    /** Hides implementation */
    struct Impl;
    /** Pointer to implementation */
    base::shared_ptr<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ResEdition Stream output */
  extern std::ostream & operator<<( std::ostream & str, const Edition & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_EDITION_H
