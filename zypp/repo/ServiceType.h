/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_SERVICE_TYPE_H_
#define ZYPP_SERVICE_TYPE_H_

#include <iosfwd>
#include <string>

namespace zypp
{
  namespace repo
  {

  /**
   * \short Service type enumeration
   *
   * Currently we have only RIS service, but more can come later.
   */
  struct ServiceType
  {
    /**
     * Repository Index Service (RIS)
     * (formerly known as 'Novell Update' (NU) service)
     */
    static const ServiceType RIS;
    /** No service set. */
    static const ServiceType NONE;
    /**
     * Plugin services are scripts installed on
     * your system that provide the package manager with
     * repositories.
     *
     * The mechanism used to create this repository list
     * is completely up to the script
     */
    static const ServiceType PLUGIN;

    enum Type
    {
      NONE_e,
      RIS_e,
      PLUGIN_e,
    };

    ServiceType() : _type(NONE_e) {}

    ServiceType(Type type) : _type(type) {}

    explicit ServiceType(const std::string & strval_r);

    Type toEnum() const { return _type; }

    ServiceType::Type parse(const std::string & strval_r);

    const std::string & asString() const;

    Type _type;
  };


  inline std::ostream & operator<<( std::ostream & str, const ServiceType & obj )
  { return str << obj.asString(); }

  inline bool operator==(const ServiceType & obj1, const ServiceType & obj2)
  { return obj1._type == obj2._type; }

  inline bool operator!=(const ServiceType & obj1, const ServiceType & obj2)
  { return ! (obj1 == obj2); }


  } // ns repo
} // ns zypp

#endif /* ZYPP_SERVICE_TYPE_H_ */

// vim: set ts=2 sts=2 sw=2 et ai:
