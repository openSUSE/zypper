/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ZYppFactory.h
 *
*/
#ifndef ZYPP_ZYPPFACTORY_H
#define ZYPP_ZYPPFACTORY_H

#include <iosfwd>

#include "zypp/ZYpp.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ZYppFactory
  //
  /** ZYpp factory class (Singleton)
  */
  class ZYppFactory
  {
    friend std::ostream & operator<<( std::ostream & str, const ZYppFactory & obj );

  public:
    /** Singleton ctor */
    static ZYppFactory instance();
    /** Dtor */
    ~ZYppFactory();

  public:
    /** \return Pointer to the ZYpp instance. */
    ZYpp::Ptr getZYpp() const;

  private:
    /** Default ctor. */
    ZYppFactory();
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ZYppFactory Stream output */
  std::ostream & operator<<( std::ostream & str, const ZYppFactory & obj );

  /** \relates ZYppFactory Convenience to get the Pointer
   * to the ZYpp instance.
  */
  inline ZYpp::Ptr getZYpp()
  { return ZYppFactory::instance().getZYpp(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ZYPPFACTORY_H
