/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Arch.cc
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * Definition of 'edition'
 *  contains epoch-version-release-arch
 *  and comparision functions
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

#include <y2util/stringutil.h>
#include <sys/utsname.h>

#include <zypp/solver/detail/Arch.h>
#include <zypp/solver/detail/utils.h>

#if 0
static char *known_archs[] = {
    "", "*", "?",			// noarch, any, unknown
    "i386", "i486", "i586", "i686", 
    "x86-64", "x86_64", "ia32e", "athlon",
    "ppc", "ppc64",
    "s390", "s390x",
    "ia64",
    "sparc", "sparc64"
};
#endif

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////
      
      using namespace std;
      
      static const string system_arch (void);
      static const string canonical_arch (const string & arch);
      
      //---------------------------------------------------------------------------
      
      const Arch * Arch::Unknown = Arch::create ("unknown");
      const Arch * Arch::Any = Arch::create ("any");
      const Arch * Arch::Noarch = Arch::create ("noarch");
      const Arch * Arch::System = Arch::create (system_arch ());
      
      //---------------------------------------------------------------------------
      // architecture stuff
      
      static const string
      canonical_arch (const string & arch)
      {
          typedef struct { char *from; char *to; } canonical;
          // convert machine string to known_arch
      static canonical canonical_archs[] = {
          { "noarch",  "noarch" },
          { "unknown", "unknown" },
          { "any",	 "any" },
          { "all",     "any" },
          { "i386",    "i386" },
          { "ix86",    "i386" }, /* OpenPKG uses this */
          { "i486",    "i486" },
          { "i586",    "i586" },
          { "i686",    "i686" },
          { "x86_64",  "x86_64" },
          { "ia32e",   "ia32e" },
          { "athlon",  "athlon" },
          { "ppc",     "ppc" },
          { "ppc64",   "ppc64" },
          { "s390",    "s390" },
          { "s390x",   "s390x" },
          { "ia64",    "ia64" },
          { "sparc",   "sparc" },
          { "sun4c",   "sparc" },
          { "sun4d",   "sparc" },
          { "sun4m",   "sparc" },
          { "sparc64", "sparc64" },
          { "sun4u",   "sparc64" },
          { "sparcv9", "sparc64" },
          { 0 }
      };
      
          for (canonical *ptr = canonical_archs; ptr->from; ptr++) {
      	if (arch == ptr->from) {
      	    return ptr->to;
      	}
          }
      
          return "canonical";
      }
      
      
      static const string
      system_arch (void)
      {
          static struct utsname buf;
          static bool checked = false;
      
          if (!checked) {
      	if (uname (&buf) < 0) {
      	    return NULL;
      	}
      	checked = true;
          }
      
          return string (buf.machine);
      }
      
      
      //---------------------------------------------------------------------------
      
      const string
      Arch::asString ( void ) const
      {
          return toString (*this);
      }
      
      
      const string
      Arch::toString ( const Arch & arch )
      {
          return arch._arch;
      }
      
      
      ostream &
      Arch::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream & os, const Arch & arch)
      {
          return os << arch.asString();
      }
      
      //---------------------------------------------------------------------------
      
      Arch::Arch( const string & a)
          : _arch (a)
      {
      }
      
      
      const Arch *
      Arch::create (const string & arch)
      {
      typedef std::map<const std::string, const Arch *> ArchTable;
      
          static ArchTable table;
          ArchTable::iterator pos = table.find (arch);
          if (pos != table.end()) {
      	return pos->second;
          }
          const Arch *new_arch = new Arch(canonical_arch (arch));
          table.insert (ArchTable::value_type (arch, new_arch));
      
          return new_arch;
      }
      
      
      ArchList
      Arch::getCompatList () const
      {
          typedef struct {
      	const char *arch;
      	const char *compat_arch;
          } ArchAndCompatArch;
      
          /* _NOARCH should never be listed in this table (other than as the
           * terminator), as it will automatically be added.  Every architecture
           * is implicitly compatible with itself.  Compatible architectures
           * should be listed in most-preferred to least-preferred order. */
      
          static ArchAndCompatArch compat_table[] = {
      	{ "i486",    "i386" },
      	{ "i586",    "i486" },
      	{ "i586",    "i386" },
      	{ "i686",    "i586" },
      	{ "i686",    "i486" },
      	{ "i686",    "i386" },
      	{ "athlon",  "i686" },
      	{ "athlon",  "i586" },
      	{ "athlon",  "i486" },
      	{ "athlon",  "i386" },
      	{ "x86_64",  "i686" },
      	{ "x86_64",  "i586" },
      	{ "x86_64",  "i486" },
      	{ "x86_64",  "i386" },
      	{ "x86_64",  "athlon" },
      	{ "x86_64",  "ia32e" },
      	{ "ppc64",   "ppc" },
      	{ "s390x",   "s390" },
      	{ "sparc64", "sparc" },
      	{ 0 }
          };
      
      
          ArchAndCompatArch *iter;
          ArchList ret;
      
          ret.push_back (this);			// be compatible with yourself
      
          iter = compat_table;
          while (iter->arch != NULL) {
              if (_arch == iter->arch) {
      	    ret.push_back (create (iter->compat_arch));
      	}
              iter++;
          }
      
          return ret;
      }
      
      
      int
      Arch::getCompatScore (const ArchList & archlist) const
      {
          int score = 0;
      
          for (ArchList::const_iterator iter = archlist.begin(); iter != archlist.end(); iter++) {
      	if (*iter == this) {
      	    return score;
      	}
              score++;
          }
          return -1;
      }
        
      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

