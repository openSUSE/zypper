/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/


#ifndef ResolvableData_h
#define ResolvableData_h

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/Pathname.h"
#include "zypp/Edition.h"
#include "zypp/ByteCount.h"
#include "zypp/Arch.h"
#include "zypp/CheckSum.h"
#include "zypp/Url.h"
#include "zypp/Date.h"
#include "zypp/TranslatedText.h"
#include <string>
#include <list>
#include <iostream>
#include <zypp/base/PtrTypes.h>

using namespace zypp::base;

namespace zypp
{
namespace data
{
  class Dependency
  {
    public:
      Dependency();
      Dependency(const std::string& kind, const std::string& encoded );
      std::string kind;
      std::string encoded;
  };
  
  typedef std::list<Dependency> DependencyList;
  
  class ResObject : public base::ReferenceCounted, private base::NonCopyable
  {
    public:
      ResObject()
        : source_media_nr(1), install_only(false)
      {}
      
      std::string name;
      Edition edition;
      Arch arch;
      
      TranslatedText summary;
      TranslatedText description;
      
      std::string insnotify;
      std::string delnotify;
      
      std::string license_to_confirm;
      std::string vendor;
      
      ByteCount size;
      ByteCount archive_size;
      
      std::string source;
      
      int source_media_nr;
      
      bool install_only;
      
      Date build_time;
      Date install_time;
      
      DependencyList provides;
      DependencyList conflicts;
      DependencyList obsoletes;
      DependencyList freshens;
      DependencyList requires;
      DependencyList prerequires;
      DependencyList recommends;
      DependencyList suggests;
      DependencyList supplements;
      DependencyList enhances;
  };
  
  class AtomBase : public ResObject
  {
    public:
      enum AtomType { TypePackage, TypeScript, TypeMessage };
      virtual AtomType atomType() = 0;
  };
  
  class Script : public AtomBase
  {
    public:
      Script() {};
      virtual AtomType atomType() { return TypeScript; };
      std::string do_script;
      std::string undo_script;
      std::string do_location;
      std::string undo_location;
      std::string do_media;
      std::string undo_media;
      std::string do_checksum_type;
      std::string do_checksum;
      std::string undo_checksum_type;
      std::string undo_checksum;
  };

  class Message : public AtomBase
  {
    public:
      Message() {};
      virtual AtomType atomType() { return TypeMessage; };
      TranslatedText text;
  };
  
  class Selection : public base::ReferenceCounted, private base::NonCopyable
  {
    public:

      Selection() {};
      std::string groupId;
      TranslatedText name;
      std::string default_;
      std::string user_visible;
      TranslatedText description;
      //std::list<MetaPkg> grouplist;
      //std::list<PackageReq> packageList;
  };

  class Patch : public ResObject
  {
    public:
      Patch() {};
  };

  class Pattern : public ResObject
  {
    public:

      Pattern() {};
      
      std::string default_;
      std::string user_visible;
      TranslatedText category;
      std::string icon;
      std::string script;
  };

  class Product : public ResObject
  {
    public:
      Product() {};
      ~Product() {};

      std::string type;
      std::string vendor;
      std::string name; 
      std::string distribution_name; 
      Edition distribution_edition;
      TranslatedText short_name;
        // those are suse specific tags
      std::string releasenotesurl;
  };

  class Package : public ResObject
  {
    public:
      Package() {};
      ~Package() {};

      std::string type;
      CheckSum checksum;
      // changlelog?
      std::string buildhost;
      std::string distribution;
      std::string license;
      std::string packager;
      std::string group;
      Url url;
      std::string os;
      
      std::string prein;
      std::string postin;
      std::string preun;
      std::string postun;
      
      ByteCount source_size;
      
      std::list<std::string> authors;
      std::list<std::string> keywords;
      
      Pathname location;
  };
  
  /* Easy output */
//   std::ostream& operator<<(std::ostream &out, const Dependency& data);
   std::ostream& operator<<(std::ostream &out, const ResObject& data);
   //std::ostream& operator<<(std::ostream &out, const Package& data);
//   std::ostream& operator<<(std::ostream &out, const Product& data);
//   std::ostream& operator<<(std::ostream &out, const Pattern& data);
//   std::ostream& operator<<(std::ostream &out, const Selection& data);
  
} // namespace data
} // namespace zypp


#endif

