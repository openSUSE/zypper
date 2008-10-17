/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/susetags/RepoIndex.h
 *
*/
#ifndef ZYPP_PARSER_SUSETAGS_REPOINDEX_H
#define ZYPP_PARSER_SUSETAGS_REPOINDEX_H

#include <iosfwd>
#include <list>
#include <map>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/Arch.h"
#include "zypp/CheckSum.h"
#include "zypp/Pathname.h"
#include "zypp/Locale.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      DEFINE_PTR_TYPE(RepoIndex);

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : RepoIndex
      //
      /** Repository content data (from /content file).
       * File and Checksum definitions required by Downloader and Parser.
       * (Do not confuse with NU's repoindex.xml)
      */
      class RepoIndex : public base::ReferenceCounted, private base::NonCopyable
      {
        friend class ContentFileReader;
	public:
	  typedef std::map<std::string, CheckSum> FileChecksumMap;

	  Pathname descrdir;
	  Pathname datadir;

	  FileChecksumMap metaFileChecksums;
          FileChecksumMap mediaFileChecksums;
	  FileChecksumMap signingKeys;

	protected:
	  /** Overload to realize std::ostream & operator\<\<. */
	  virtual std::ostream & dumpOn( std::ostream & str ) const;
      };
      //////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
    } // namespace susetags
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_SUSETAGS_REPOINDEX_H
