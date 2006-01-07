/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* PackageUpdate.h
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

#ifndef ZYPP_SOLVER_TEMPORARY_PACKAGEUPDATE_H
#define ZYPP_SOLVER_TEMPORARY_PACKAGEUPDATE_H

#include <list>
#include <iosfwd>
#include <string>
#include <sys/types.h>

#include "zypp/solver/detail/Importance.h"

#include "zypp/solver/temporary/PackageUpdatePtr.h"
#include "zypp/solver/temporary/PackagePtr.h"
#include "zypp/solver/temporary/Spec.h"
#include "zypp/solver/temporary/XmlNode.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : PackageUpdate
/**
 *
 **/

class PackageUpdate : public Spec {
          
      
        private:
          Package_Ptr _package;
      
          const char *_package_url;
          size_t _package_size;
          size_t _installed_size;
      
          const char *_signature_url;
          size_t _signature_size;
      
          const char *_md5sum;
      
          const Importance *_importance;
      
          unsigned int _hid;
      
          const char *_description;
      
          const char *_license;
      
          // refers to the parent package for SuSE patch RPMs
          Package_constPtr _parent;
      
        public:
      
          PackageUpdate(const std::string & name);
          PackageUpdate(XmlNode_constPtr node, Package_Ptr package);
      
          virtual ~PackageUpdate();
      
          // ---------------------------------- I/O
      
          const XmlNode_Ptr asXmlNode (void) const;
      
          static std::string toString ( const PackageUpdate & packageupdate, bool full = false );
      
          virtual std::ostream & dumpOn( std::ostream & str ) const;
      
          friend std::ostream& operator<<( std::ostream&, const PackageUpdate& );
      
          std::string asString ( bool full = false ) const;
      
          // ---------------------------------- accessors
      
          Package_constPtr package() const { return _package; }
          void setPackage (Package_Ptr package) { _package = package; }
      
          const char *packageUrl() const { return _package_url; }
          void setPackageUrl (const char *package_url) { _package_url = package_url; }
      
          size_t packageSize() const { return _package_size; }
          void setPackageSize (size_t package_size) { _package_size = package_size; }
      
          size_t installedSize() const { return _installed_size; }
          void setInstalledSize (size_t installed_size ) { _installed_size = installed_size; }
      
          const char *description() const { return _description; }
          void setDescription(const char *description) { _description = strdup (description); }
      
          const char *signatureUrl() const { return _signature_url; }
          void setSignatureUrl (const char *signature_url) { _signature_url = signature_url; }
      
          size_t signatureSize() const { return _signature_size; }
          void setSignatureSize (size_t signature_size) { _signature_size = signature_size; }
      
          const char *md5sum() const { return _md5sum; }
          void setMd5sum (const char *md5sum) { _md5sum = md5sum; }
      
          const Importance *importance() const { return _importance; }
          void setImportance (const Importance *importance) { _importance = importance; }
      
          unsigned int hid() const { return _hid; }
          void setHid (unsigned int hid) { _hid = hid; }
      
          const char *license() const { return _license; }
          void setLicense (const char *license) { _license = license; }
      
          // refers to the parent package for SuSE patch RPMs
          Package_constPtr parent() const { return _parent; }
          void setParent (Package_constPtr parent) { _parent = parent; }
      
          // ---------------------------------- methods
      
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
#endif // ZYPP_SOLVER_TEMPORARY_PACKAGEUPDATE_H
