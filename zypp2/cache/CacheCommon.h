/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef CACHE_COMMON_H
#define CACHE_COMMON_H

//-----------------------------------------------------------------------------
// relations

#define RELATION_ANY 0
#define RELATION_EQUAL (1 << 0)
#define RELATION_LESS (1 << 1)
#define RELATION_GREATER (1 << 2)
#define RELATION_NONE (1 << 3)

namespace zypp
{
namespace cache
{

namespace db
{
  
typedef enum {
  REL_INVALID            = -1,
  REL_ANY                = RELATION_ANY,
  REL_EQUAL              = RELATION_EQUAL,
  REL_LESS               = RELATION_LESS,
  REL_LESS_EQUAL         = RELATION_LESS | RELATION_EQUAL,
  REL_GREATER            = RELATION_GREATER,
  REL_GREATER_EQUAL      = RELATION_GREATER | RELATION_EQUAL,
  REL_NOT_EQUAL          = RELATION_LESS | RELATION_GREATER,
  REL_NONE               = RELATION_NONE,
} Rel;

//-----------------------------------------------------------------------------
// architectures

typedef enum {
  ARCH_UNKNOWN = -1,
  ARCH_NOARCH = 0,
  ARCH_I386,
  ARCH_I486,
  ARCH_I586,
  ARCH_I686,
  ARCH_X86_64,
  ARCH_IA32E,
  ARCH_ATHLON,
  ARCH_PPC,
  ARCH_PPC64,
  ARCH_S390,
  ARCH_S390X,
  ARCH_IA64,
  ARCH_SPARC,
  ARCH_SPARC64,
} Arch;

//-----------------------------------------------------------------------------
// dependencies

typedef enum {
  DEP_TYPE_REQUIRE = 0,
  DEP_TYPE_PROVIDE,			// 1
  DEP_TYPE_CONFLICT,			// 2
  DEP_TYPE_OBSOLETE,			// 3
  DEP_TYPE_PREREQUIRE,			// 4
  DEP_TYPE_FRESHEN,			// 5
  DEP_TYPE_RECOMMEND,			// 6
  DEP_TYPE_SUGGEST,			// 7
  DEP_TYPE_SUPPLEMENT,			// 8
  DEP_TYPE_ENHANCE,		// 9
  DEP_TYPE_UNKNOWN=420
} DependencyType;

//-----------------------------------------------------------------------------
// kinds (dependencies.dep_target

typedef enum {
  KIND_PACKAGE = 0,
  KIND_SCRIPT,			// 1
  KIND_MESSAGE,			// 2
  KIND_PATCH,			// 3
  KIND_PATTERN,			// 4
  KIND_PRODUCT,			// 5
  KIND_SELECTION,		// 6
  KIND_LANGUAGE,			// 7
  KIND_ATOM,			// 8
  KIND_SRC,			// 9
  KIND_SYSTEM,			// 10 SystemResObject
  KIND_UNKNOWN=42		// 42
} Kind;


} // namespace db
} //namespace cache
} //namespace zypp

#endif // CACHE_COMMON_H
