/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Package.h
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _Package_h
#define _Package_h

#include <list>
#include <iosfwd>
#include <string>
#include <sys/types.h>

#include "zypp/solver/detail/PackagePtr.h"
#include "zypp/solver/detail/PackageUpdatePtr.h"
#include "zypp/solver/detail/Section.h"
#include "zypp/solver/detail/ResItem.h"
#include "zypp/solver/detail/XmlNode.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

typedef std::list<Package_Ptr> PackageList;
typedef PackageList * PackageList_Ptr;

typedef std::list<PackageUpdate_Ptr> PackageUpdateList;
typedef PackageUpdateList * PackageUpdateList_Ptr;


///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : Package
/**
 *
 **/

class Package : public ResItem {
    
      
        private:
          Section_Ptr _section;
          
          // Filled in by package info XML
          std::string _pretty_name;
          std::string _summary;
          std::string _description;
      
          PackageUpdateList _history;
      
          // After downloading this package, fill in the local file name,
          //   and signature, if appropriate
          std::string _package_filename;
          std::string _signature_filename;
      
          bool _install_only;		// Only install, don't upgrade this package
          bool _package_set;
      
          std::string _id;
      
        public:
      
          Package(Channel_constPtr channel);
          Package(Channel_constPtr channel,
                  const std::string & name = "",
                  const Edition & edition = Edition::noedition,
                  const Arch arch = Arch_noarch);
          Package(XmlNode_constPtr node, Channel_constPtr channel);	//RCPackage *rc_xml_node_to_package (const xmlNode *node, const RCChannel *channel);
          virtual ~Package();
      
          // ---------------------------------- I/O
      
          const xmlNodePtr asXmlNode (void) const;				// xmlNode *rc_package_to_xml_node (RCPackage *package);
      
          static std::string toString ( const Package & spec, bool full = false );
      
          static std::string toString ( const PackageUpdateList & l, bool full = false );
      
          virtual std::ostream & dumpOn( std::ostream & str ) const;
      
          friend std::ostream& operator<<( std::ostream&, const Package& );
      
          std::string asString ( bool full = false ) const;
      
          // ---------------------------------- accessors
      
          // accessor for _section
          const Section_Ptr section() const { return _section; }
          void setSection (const Section_Ptr section) { _section = section; }
      
          // accessor for _pretty_name
          const std::string prettyName() const { return _pretty_name; }
          void setPrettyName(const std::string & pretty_name) { _pretty_name = pretty_name; }
      
          // accessor for _summary
          const std::string summary() const { return _summary; }
          void setSummary (const std::string & summary) { _summary = summary; }
      
          // accessor for _description
          const std::string description() const { return _description; }
          void setDescription(const std::string & description) { _description = description; }
      
          // accessor for _package_filename
          const PackageUpdateList & history() const { return _history; }
          void setHistory(const PackageUpdateList & history) { _history = history; }
      
          // accessor for _package_filename
          const std::string packageFilename() const { return _package_filename; }
          void setPackageFilename(const std::string & package_filename) { _package_filename = package_filename; }
      
          // accessor for _signature_filename
          const std::string signatureFilename() const { return _signature_filename; }
          void setSignatureFilename(const std::string & signature_filename) { _signature_filename = signature_filename; }
      
          // accessor for _install_only
          bool installOnly() const { return _install_only; }
          void setInstallOnly(bool install_only) { _install_only = install_only; }
      
          // accessor for _package_set
          bool packageSet() const { return _package_set; }
          void setPackageSet(bool package_set) { _package_set = package_set; }
      
          // accessor for id
          const std::string id() const { return _id; }
          void setId (const std::string & id) { _id = id; }
      
          // ---------------------------------- methods
      
          void addUpdate (PackageUpdate_Ptr update);
      
          PackageUpdate_Ptr getLatestUpdate (void) const;
      };

      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////
#endif // _Package_h
