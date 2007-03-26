#include "zypp2/cache/DatabaseTypes.h"

using namespace std;
using namespace zypp;
using namespace zypp::cache;

namespace zypp
{
namespace cache
{

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
zypp_kind2db_kind( Resolvable::Kind kind )
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

//----------------------------------------------------------------------------
// convert ZYPP Resolvable kind to ZMD db::Kind

Resolvable::Kind
db_kind2zypp_kind( db::Kind kind )
{
  switch ( kind )
  {
    case db::KIND_PACKAGE:
      return ResTraits<Package>::kind;
    case db::KIND_SCRIPT:
      return ResTraits<Script>::kind;
    case db::KIND_MESSAGE:
      return ResTraits<Message>::kind;
    case db::KIND_PATCH:
      return ResTraits<Selection>::kind;
    case db::KIND_PATTERN:
      return ResTraits<Pattern>::kind;
    case db::KIND_PRODUCT:
      return ResTraits<Product>::kind;
    case db::KIND_LANGUAGE:
      return ResTraits<Language>::kind;
    case db::KIND_ATOM:
      return ResTraits<Atom>::kind;
    case db::KIND_SRC:
      return ResTraits<SrcPackage>::kind;
    case db::KIND_SYSTEM:
      return ResTraits<SystemResObject>::kind;
  }
  // unreached
}


db::DependencyType
zypp_deptype2db_deptype( zypp::Dep deptype )
{
  switch ( deptype.inSwitch() )
  {
    case zypp::Dep::PROVIDES_e: return db::DEP_TYPE_PROVIDE;
    case zypp::Dep::CONFLICTS_e: return db::DEP_TYPE_CONFLICT;
    case zypp::Dep::OBSOLETES_e: return db::DEP_TYPE_OBSOLETE;
    case zypp::Dep::FRESHENS_e: return db::DEP_TYPE_FRESHEN;
    case zypp::Dep::REQUIRES_e: return db::DEP_TYPE_REQUIRE;
    case zypp::Dep::PREREQUIRES_e: return db::DEP_TYPE_PREREQUIRE;
    case zypp::Dep::RECOMMENDS_e: return db::DEP_TYPE_RECOMMEND;
    case zypp::Dep::SUGGESTS_e: return db::DEP_TYPE_SUGGEST;
    case zypp::Dep::SUPPLEMENTS_e: return db::DEP_TYPE_SUPPLEMENT;
    case zypp::Dep::ENHANCES_e: return db::DEP_TYPE_ENHANCE;
    default:
    return db::DEP_TYPE_UNKNOWN;
   }
   
}

zypp::Dep
db_deptype2zypp_deptype( db::DependencyType deptype )
{
  switch ( deptype )
  {
    case db::DEP_TYPE_PROVIDE: return zypp::Dep::PROVIDES; break;
    case db::DEP_TYPE_CONFLICT: return zypp::Dep::CONFLICTS; break;
    case db::DEP_TYPE_OBSOLETE: return zypp::Dep::OBSOLETES; break;
    case db::DEP_TYPE_FRESHEN: return zypp::Dep::FRESHENS; break;
    case db::DEP_TYPE_REQUIRE: return zypp::Dep::REQUIRES; break;
    case db::DEP_TYPE_PREREQUIRE: return zypp::Dep::PREREQUIRES; break;
    case db::DEP_TYPE_RECOMMEND: return zypp::Dep::RECOMMENDS; break;
    case db::DEP_TYPE_SUGGEST: return zypp::Dep::SUGGESTS; break;
    case db::DEP_TYPE_SUPPLEMENT: return zypp::Dep::SUPPLEMENTS; break;
    case db::DEP_TYPE_ENHANCE: return zypp::Dep::ENHANCES; break;
   }
}

} // ns cache
} // ns zypp

