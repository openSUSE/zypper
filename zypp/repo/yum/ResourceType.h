/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef YUMRESOURCETYPE_H_
#define YUMRESOURCETYPE_H_

#include <iosfwd>
#include<string>

namespace zypp
{
  namespace repo
  {
    namespace yum
    {


  /**
   *
   */
  struct ResourceType
  {
    static const ResourceType NONE; // unknown
    static const ResourceType REPOMD;
    static const ResourceType PRIMARY;
    static const ResourceType OTHER;
    static const ResourceType FILELISTS;
    static const ResourceType GROUP;
    static const ResourceType PATCHES; // suse extension
    static const ResourceType PATCH;   // suse extension
    static const ResourceType PRODUCT; // suse extension
    static const ResourceType PATTERNS; // suse extension
    // sqlite caches yum extensions:
    static const ResourceType PRIMARY_DB; // yum extension
    static const ResourceType OTHER_DB; // yum extension

    enum Type
    {
      NONE_e,
      REPOMD_e,
      PRIMARY_e,
      OTHER_e,
      FILELISTS_e,
      GROUP_e,
      PATCHES_e,
      PATCH_e,
      PRODUCT_e,
      PATTERNS_e,
      PRIMARY_DB_e,
      OTHER_DB_e,
    };

    ResourceType(Type type) : _type(type) {}

    explicit ResourceType(const std::string & strval_r);

    Type toEnum() const { return _type; }

    ResourceType::Type parse(const std::string & strval_r);

    const std::string & asString() const;

    Type _type;
  };


  inline std::ostream & operator<<( std::ostream & str, const ResourceType & obj )
  { return str << obj.asString(); }

  inline bool operator==(const ResourceType & obj1, const ResourceType & obj2)
  { return obj1._type == obj2._type; }


    } // ns yum
  } // ns source
} // ns zypp

#endif /*YUMRESOURCETYPE_H_*/

// vim: set ts=2 sts=2 sw=2 et ai:
