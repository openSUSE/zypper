/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_REPO_TYPE_H_
#define ZYPP_REPO_TYPE_H_

#include <iosfwd>
#include <string>

namespace zypp
{
  namespace repo
  {

  /**
   * \short Repository type enumeration
   *
   * Repositories can be from varous types
   * ...
   */
  struct RepoType
  {
    static const RepoType RPMMD;
    static const RepoType YAST2;
    static const RepoType RPMPLAINDIR;
    static const RepoType NONE;

    enum Type
    {
      NONE_e,
      RPMMD_e,
      YAST2_e,
      RPMPLAINDIR_e,
    };

    RepoType() : _type(NONE_e) {}

    RepoType(Type type) : _type(type) {}

    explicit RepoType(const std::string & strval_r);

    Type toEnum() const { return _type; }

    RepoType::Type parse(const std::string & strval_r);

    const std::string & asString() const;

    Type _type;
  };


  inline std::ostream & operator<<( std::ostream & str, const RepoType & obj )
  { return str << obj.asString(); }

  inline bool operator==(const RepoType & obj1, const RepoType & obj2)
  { return obj1._type == obj2._type; }

  inline bool operator!=(const RepoType & obj1, const RepoType & obj2)
  { return ! (obj1 == obj2); }

  } // ns repo
} // ns zypp

#endif

// vim: set ts=2 sts=2 sw=2 et ai:
