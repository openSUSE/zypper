/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xmltore/schemanames.h
 *
*/


#ifndef xmlstore_schemanames_h
#define xmlstore_schemanames_h

namespace zypp {
  namespace parser {
    namespace xmlstore {
    /* FIXME: How do I do this properly? */
#define SCHEMABASE "/usr/share/zypp/schema/xmlstore/"
    //#define REPOMDSCHEMA (SCHEMABASE "repomd.rng")
    //#define PRIMARYSCHEMA (SCHEMABASE "suse-primary.rng")
    //#define GROUPSCHEMA (SCHEMABASE "groups.rng")
    #define PATTERNSCHEMA (SCHEMABASE "pattern.rng")
    //#define FILELISTSCHEMA (SCHEMABASE "filelists.rng")
    //#define OTHERSCHEMA (SCHEMABASE "other.rng")
    #define PATCHSCHEMA (SCHEMABASE "patch.rng")
    //#define PATCHESSCHEMA (SCHEMABASE "patches.rng")
    //#define PRODUCTSCHEMA (SCHEMABASE "product.rng")
    } // namespace xmlstore
  } // namespace parser
} // namespace zypp

#endif
