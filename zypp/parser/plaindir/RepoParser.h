/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_PARSER_PLAINDIR_REPOPARSER_H
#define ZYPP_PARSER_PLAINDIR_REPOPARSER_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/NonCopyable.h"

#include "zypp/data/RecordId.h"
#include "zypp/data/ResolvableDataConsumer.h"

#include "zypp/ProgressData.h"
#include "zypp/RepoStatus.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace plaindir
    { /////////////////////////////////////////////////////////////////

      /**
       * \short Gives a cookie for a dir
       */
      RepoStatus dirStatus( const Pathname &dir );
      
      /** Plaindir metadata parser. */
       
      class RepoParser : private base::NonCopyable
      {
      public:
       /** Ctor.
        *
        * \param repositoryId_r repository identifier
        * \param consumer_r consumer of parsed data
        * \param fnc_r progress reporting function
	*/
	RepoParser( const std::string & repositoryId_r,
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
    } // namespace plaindir
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP2_PARSER_SUSETAGS_REPOPARSER_H
