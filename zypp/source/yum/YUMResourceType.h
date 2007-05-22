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
  namespace source
  {
    namespace yum
    {


  /**
   * 
   */
  struct YUMResourceType
  {
    static const YUMResourceType REPOMD;
    static const YUMResourceType PRIMARY;
    static const YUMResourceType OTHER;
    static const YUMResourceType FILELISTS;
    static const YUMResourceType GROUP;
    static const YUMResourceType PATCHES; // suse extension
    static const YUMResourceType PATCH;   // suse extension
    static const YUMResourceType PRODUCT; // suse extension
    static const YUMResourceType PATTERN; // suse extension

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
      PATTERN_e
    };

    YUMResourceType(Type type) : _type(type) {}

    explicit YUMResourceType(const std::string & strval_r);

    const Type toEnum() const { return _type; }
    
    YUMResourceType::Type parse(const std::string & strval_r);

    const std::string & asString() const;

    Type _type;
  };


  inline std::ostream & operator<<( std::ostream & str, const YUMResourceType & obj )
  { return str << obj.asString(); }

  inline bool operator==(const YUMResourceType & obj1, const YUMResourceType & obj2)
  { return obj1._type == obj2._type; }


    } // ns yum
  } // ns source
} // ns zypp

#endif /*YUMRESOURCETYPE_H_*/

// vim: set ts=2 sts=2 sw=2 et ai:
