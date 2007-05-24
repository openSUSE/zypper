/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp2/parser/susetags/RepoParser.h
 *
*/
#ifndef ZYPP2_PARSER_SUSETAGS_REPOPARSER_H
#define ZYPP2_PARSER_SUSETAGS_REPOPARSER_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/NonCopyable.h"

#include "zypp/data/RecordId.h"
#include "zypp/data/ResolvableDataConsumer.h"

#include "zypp/ProgressData.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : RepoParser
      //
      /** SuseTags metadata parser.
       *
       * Reads a \c content file to get the \ref data::Product and a \Ref RepoIndex.
       * Then parses the remaining files and feeds them to a \ref data::ResolvableDataConsumer
       * (typically to store them in a database).
       *
       * \see \ref ContentFileReader and \ref FileReaderBase
       *
       * \code
       *   Pathname dbdir( "store" );
       *   Pathname reporoot( "lmd" );
       *
       *   cache::CacheStore store( dbdir );
       *   data::RecordId catalogId = store.lookupOrAppendRepository( Url("dir:///somewhere"), "/" );
       *
       *   parser::susetags::RepoParser repo( catalogId, store );
       *   repo.parse( reporoot );
       *
       *   store.commit();
       * \endcode
       *
       * \todo Improve selection of Languages to parse
       * \todo Improve feeding of translations into Cachestore. Add specialized consumer, for Du too.
       * \todo DiskUsage filereader and parsing
       * \todo more doc and Exception specification
       */
      class RepoParser : private base::NonCopyable
      {
      public:
       /** Ctor.
        *
        * \param repositoryId_r repository identifier
        * \param consumer_r consumer of parsed data
        * \param fnc_r progress reporting function
	*/
	RepoParser( const data::RecordId & repositoryId_r,
		    data::ResolvableDataConsumer & consumer_r,
		    const ProgressData::ReceiverFnc & fnc_r = ProgressData::ReceiverFnc() );
        /** Dtor */
        ~RepoParser();

       /** Parse a local repository located at \a reporoot_r.
        *
        * \param reporoot_r The local repositories root directory.
	* \throw Exception on errors.
	*/
	void parse( const Pathname & reporoot_r );

	public:
	  class Impl;
	private:
	  RW_pointer<Impl,rw_pointer::Scoped<Impl> > _pimpl;
      };
      ///////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
    } // namespace susetags
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP2_PARSER_SUSETAGS_REPOPARSER_H
