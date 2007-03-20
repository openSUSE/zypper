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
#include "zypp/capability/CapabilityImpl.h"
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
  typedef std::list< capability::CapabilityImpl::Ptr > DependencyList;

  struct Dependencies
  {
//     DependencyList & operator[]( zypp::Dep idx )
//     {
//       switch ( idx.inSwitch() )
//       {
//         case zypp::Dep::PROVIDES.inSwitch(): return provides; break;
//         case zypp::Dep::CONFLICTS.inSwitch(): return conflicts; break;
//         case zypp::Dep::OBSOLETES.inSwitch(): return obsoletes; break;
//         case zypp::Dep::FRESHENS.inSwitch(): return freshens; break;
//         case zypp::Dep::REQUIRES.inSwitch(): return requires; break;
//         case zypp::Dep::PREREQUIRES.inSwitch(): return prerequires; break;
//         case zypp::Dep::RECOMMENDS.inSwitch(): return recommends; break;
//         case zypp::Dep::SUGGESTS.inSwitch(): return suggests; break;
//         case zypp::Dep::SUPPLEMENTS.inSwitch(): return supplements; break;
//         case zypp::Dep::ENHANCES.inSwitch(): return enhances; break;
//       }
//     }
    
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
  
  class Resolvable : public base::ReferenceCounted, private base::NonCopyable
  {
    public:
    Resolvable()
    {};
    
    std::string name;
    Edition edition;
    Arch arch;
    
    Dependencies deps;
  };
  
  struct ResObjectData
  {
      ResObjectData()
        : source_media_nr(1), install_only(false)
      {}
      
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
  };
  
  
  class ResObject : public Resolvable
  {
    public:
      ResObject()
        : source_media_nr(1), install_only(false)
      {}
      
      
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

  /*
   * Data Object for Pattern
   * resolvable
   */
  class Pattern : public ResObject
  {
    public:

      Pattern()
        : user_visible(true)
      {};
      
      std::string default_;
      bool user_visible;
      TranslatedText category;
      std::string icon;
      std::string script;
  };

  /*
   * Data Object for Product
   * resolvable
   */
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

    /*
   * Data Object for Package
   * resolvable
   */
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

