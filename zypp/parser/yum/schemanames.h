/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

File:       schemanames.h

Author:     Michael Radziej <mir@suse.de>
Maintainer: Michael Radziej <mir@suse.de>

Purpose:    Pathnames of schemas for validation

/-*/

#ifndef schemanames_h
#define schemanames_h

namespace zypp { namespace parser { namespace YUM {
/* FIXME: How do I do this properly? */
#define SCHEMABASE "/usr/share/YaST2/schema/packagemanager/"
#define REPOMDSCHEMA (SCHEMABASE "repomd.rng")
#define PRIMARYSCHEMA (SCHEMABASE "suse-primary.rng")
#define GROUPSCHEMA (SCHEMABASE "groups.rng")
#define FILELISTSCHEMA (SCHEMABASE "filelists.rng")
#define OTHERSCHEMA (SCHEMABASE "other.rng")
#define PATCHSCHEMA (SCHEMABASE "patch.rng")
}}}


#endif
