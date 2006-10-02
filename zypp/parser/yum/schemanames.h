/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/schemanames.h
 *
*/


#ifndef schemanames_h
#define schemanames_h

namespace zypp
{
namespace parser
{
namespace yum
{
/* FIXME: How do I do this properly? */
#define SCHEMABASE "/usr/share/zypp/schema/yum/"
#define REPOMDSCHEMA (SCHEMABASE "repomd.rng")
#define PRIMARYSCHEMA (SCHEMABASE "suse-primary.rng")
#define GROUPSCHEMA (SCHEMABASE "groups.rng")
#define PATTERNSCHEMA (SCHEMABASE "patterns.rng")
#define FILELISTSCHEMA (SCHEMABASE "filelists.rng")
#define OTHERSCHEMA (SCHEMABASE "other.rng")
#define PATCHSCHEMA (SCHEMABASE "patch.rng")
#define PATCHESSCHEMA (SCHEMABASE "patches.rng")
#define PRODUCTSCHEMA (SCHEMABASE "product.rng")
} // namespace yum
} // namespace parser
} // namespace zypp


#endif
