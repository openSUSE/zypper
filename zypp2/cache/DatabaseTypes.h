#include "zypp2/cache/CacheCommon.h"
#include "zypp/NeedAType.h"
#include "zypp/Resolvable.h"
#include "zypp/base/Logger.h"
#include "zypp/Arch.h"
#include "zypp/Rel.h"

using namespace std;
using namespace zypp;
using namespace zypp::cache;

static struct archrc
{
  char *arch;
  db::Arch rc;
}
archtable[] = {
                { "noarch",	db::ARCH_NOARCH },
                { "i386",	db::ARCH_I386 },
                { "i486",	db::ARCH_I486 },
                { "i586",	db::ARCH_I586 },
                { "i686",	db::ARCH_I686 },
                { "x86_64",	db::ARCH_X86_64 },
                { "ia32e",	db::ARCH_IA32E },
                { "athlon",	db::ARCH_ATHLON },
                { "ppc",	db::ARCH_PPC },
                { "ppc64",	db::ARCH_PPC64 },
                { "s390",	db::ARCH_S390 },
                { "s390x",	db::ARCH_S390X },
                { "ia64",	db::ARCH_IA64 },
                { "sparc",	db::ARCH_SPARC },
                { "sparc64",	db::ARCH_SPARC64 },
                { NULL,		db::ARCH_UNKNOWN }
              };

//----------------------------------------------------------------------------

// Convert ZYPP relation operator to ZMD RCResolvableRelation

db::Rel
zypp_rel2db_rel( zypp::Rel op)
{

  /* This enum is here so that gdb can give us pretty strings */

  switch (op.inSwitch())
  {
  case Rel::EQ_e:
    return db::REL_EQUAL;
    break;
  case Rel::NE_e:
    return db::REL_NOT_EQUAL;
    break;
  case Rel::LT_e:
    return db::REL_LESS;
    break;
  case Rel::LE_e:
    return db::REL_LESS_EQUAL;
    break;
  case Rel::GT_e:
    return db::REL_GREATER;
    break;
  case Rel::GE_e:
    return db::REL_GREATER_EQUAL;
    break;
  case Rel::ANY_e:
    return db::REL_ANY;
    break;
  case Rel::NONE_e:
    return db::REL_NONE;
    break;
  }
  return db::REL_INVALID;
}


Rel
db_rel2zypp_rel (db::Rel rel)
{
  switch (rel)
  {
  case db::REL_INVALID:
    return Rel::NONE;
    break;
  case db::REL_ANY:
    return Rel::ANY;
    break;
  case db::REL_EQUAL:
    return Rel::EQ;
    break;
  case db::REL_LESS:
    return Rel::LT;
    break;
  case db::REL_LESS_EQUAL:
    return Rel::LE;
    break;
  case db::REL_GREATER:
    return Rel::GT;
    break;
  case db::REL_GREATER_EQUAL:
    return Rel::GE;
    break;
  case db::REL_NOT_EQUAL:
    return Rel::NE;
    break;
  case db::REL_NONE:
    return Rel::ANY;
    break;
  }
  return Rel::NONE;
}

//----------------------------------------------------------------------------

// convert ZYPP architecture string to ZMD int

db::Arch
zypp_arch2db_arch(const Arch & arch)
{
  string arch_str = arch.asString();
  struct archrc *aptr = archtable;
  while (aptr->arch != NULL)
  {
    if (arch_str == aptr->arch)
      break;
    aptr++;
  }

  return aptr->rc;
}

Arch
db_arch2zypp_arch (db::Arch rc)
{
  if (rc == db::ARCH_UNKNOWN)
    return Arch();

  struct archrc *aptr = archtable;
  while (aptr->arch != NULL)
  {
    if (aptr->rc == rc)
    {
      return Arch (aptr->arch);
    }
    aptr++;
  }
  WAR << "DbAccess::Rc2Arch(" << rc << ") unknown" << endl;
  return Arch ();
}


// remove Authors from description Text
string desc2str (const Text t)
{
  static string s;		// static so we can use sqlite STATIC below
  s.clear();
  string::size_type authors = t.find ("Authors:");		// strip off 'Authors:'

  if (authors == string::npos)
  {	// if no "Authors", point to end of string
    authors = t.size();
  }

  // now remove trailing whitespace

  do
  {
    --authors;
  }
  while (t[authors] == ' ' || t[authors] == '\n');
  s = string( t, 0, authors+1 );

  return s;
}

//----------------------------------------------------------------------------
// convert ZYPP Resolvable kind to ZMD db::Kind

db::Kind
db_kind2zypp_kind( Resolvable::Kind kind )
{
  if (kind == ResTraits<Package>::kind)	 return db::KIND_PACKAGE;
  else if (kind == ResTraits<Script>::kind)	 return db::KIND_SCRIPT;
  else if (kind == ResTraits<Message>::kind)	 return db::KIND_MESSAGE;
  else if (kind == ResTraits<Patch>::kind)	 return db::KIND_PATCH;
  else if (kind == ResTraits<Selection>::kind) return db::KIND_SELECTION;
  else if (kind == ResTraits<Pattern>::kind)	 return db::KIND_PATTERN;
  else if (kind == ResTraits<Product>::kind)	 return db::KIND_PRODUCT;
  else if (kind == ResTraits<Language>::kind)	 return db::KIND_LANGUAGE;
  else if (kind == ResTraits<Atom>::kind)	 return db::KIND_ATOM;
  else if (kind == ResTraits<SrcPackage>::kind) return db::KIND_SRC;
  else if (kind == ResTraits<SystemResObject>::kind) return db::KIND_SYSTEM;

  WAR << "Unknown resolvable kind " << kind << endl;
  return db::KIND_UNKNOWN;
}


