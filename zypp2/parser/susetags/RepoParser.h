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
       * \todo more doc and Exception specification
       */
      class RepoParser : private base::NonCopyable
      {
      public:
       /** Ctor.
        *
        * \param catalogId_r repository identifier
        * \param consumer_r consumer of parsed data
        * \param fnc_r progress reporting function
	*/
	RepoParser( const data::RecordId & catalogId_r,
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
