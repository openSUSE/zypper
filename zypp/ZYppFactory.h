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
    ZYppFactoryException( const std::string & msg_r, pid_t lockerPid_r, const std::string & lockerName_r );
    virtual ~ZYppFactoryException() throw ();
  public:
    pid_t lockerPid() const { return _lockerPid; }
    const std::string & lockerName() const { return _lockerName; }
  private:
    pid_t _lockerPid;
    std::string _lockerName;
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
