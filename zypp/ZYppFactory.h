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

#include "zypp/base/Exception.h"
#include "zypp/ZYpp.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class ZYppFactoryException : public Exception
  {
  public:
    ZYppFactoryException( const std::string & msg_r, pid_t locker_pid );
    pid_t locker_pid() const { return _locker_pid; }
  private:
    pid_t _locker_pid;
  };

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
    /** \return Pointer to the ZYpp instance.
     * \throw EXCEPTION In case we can't acquire a lock.
    */
    ZYpp::Ptr getZYpp() const;

    /** Whether the ZYpp instance is already created.*/
    bool haveZYpp() const;

  private:
    /** Default ctor. */
    ZYppFactory();
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ZYppFactory Stream output */
  std::ostream & operator<<( std::ostream & str, const ZYppFactory & obj );

  /** \relates ZYppFactory Convenience to get the Pointer
   * to the ZYpp instance.
   * \see ZYppFactory::getZYpp
  */
  inline ZYpp::Ptr getZYpp()
  { return ZYppFactory::instance().getZYpp(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ZYPPFACTORY_H
