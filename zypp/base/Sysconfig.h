/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/Sysconfig.h
 *
*/
#ifndef ZYPP_BASE_SYSCONFIG_H
#define ZYPP_BASE_SYSCONFIG_H

#include <string>
#include <map>
#include "zypp/Pathname.h"

namespace zypp {
  namespace base {
    namespace sysconfig {

      /** Read sysconfig file \a path_r and return <tt>(key,valye)</tt> pairs. */
      std::map<std::string,std::string> read( const Pathname & _path );

      /** Add or change a value in sysconfig file \a path_r.
       *
       * If \a key_r already exists, only the \a val_r is changed accordingly.
       *
       * In case \a key_r is not yet present in the file, a new entry may be created
       * at the end of the file, using the lines in \a newcomment_r as comment
       * block. If \a newcomment_r is not provided or empty, a new value is not
       * created and \c false is returned.
       *
       * \returns \c TRUE if an entry was changed or created.
       *
       * \throws Exception if \a path_r can not be read or written.
       *
       * \note \a val_r is written as it is. The caller is responsible for escaping and
       * enclosing in '"', in case this is needed (\see \ref writeStringVal and \ref str::escape).
       *
       * \note Lines in \a newcomment_r which do not already start with a '#',
       * are prefixes with "# ".
       *
       * \code
       *  ## Type: string
       *  ## Default: ""
       *  #
       *  # A multiline description of
       *  # the options purpose.
       *  #
       *  KEY="value"
       * \endcode
       */
      bool write( const Pathname & path_r, const std::string & key_r, const std::string & val_r,
		  const std::string & newcomment_r = std::string() );

      /** Convenience to add or change a string-value in sysconfig file \a path_r.
       *
       * \a val_r is expected to be a plain string value, so it is propery escaped and enclosed in
       * double quotes before it is written to the sysconfig file \a path_r.
       *
       * \see \ref write
       */
      bool writeStringVal( const Pathname & path_r, const std::string & key_r, const std::string & val_r,
			   const std::string & newcomment_r = std::string() );

    } // namespace sysconfig
  } // namespace base
} // namespace zypp

#endif // ZYPP_BASE_SYSCONFIG_H
