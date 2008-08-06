/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/susetags/ContentFileReader.h
 *
*/
#ifndef ZYPP_PARSER_SUSETAGS_CONTENTFILEREADER_H
#define ZYPP_PARSER_SUSETAGS_CONTENTFILEREADER_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/Function.h"
#include "zypp/base/InputStream.h"

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

      class RepoIndex;
      DEFINE_PTR_TYPE(RepoIndex);

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : ContentFileReader
      //
      /** Parse repoindex part from a \c content file.
       * This is all the downloader needs.
      */
      class ContentFileReader : private base::NonCopyable
      {
	public:
	  typedef function<void(const RepoIndex_Ptr &)> RepoIndexConsumer;

	public:
	  /** Default ctor */
	  ContentFileReader();
	  /** Dtor */
	  virtual ~ContentFileReader();
          /** Parse the stream.
	   * \throw ParseException on errors.
	   * \throw AbortRequestException on user request.
	   * Invokes \ref consume for each tag. \ref consume might throw
	   * other exceptions as well.
	   */
	  virtual void parse( const InputStream & imput_r,
			      const ProgressData::ReceiverFnc & fnc_r = ProgressData::ReceiverFnc() );

	public:
	  /** Consumer to call when repo index was parsed. */
	  void setRepoIndexConsumer( const RepoIndexConsumer & fnc_r )
	  { _repoIndexConsumer = fnc_r; }

	protected:
	  /** Called when start parsing. */
	  virtual void beginParse();
	  /** Called when the parse is done. */
	  virtual void endParse();

	protected:
          /** Called when user(callback) request to abort.
	   * \throws AbortRequestException unless overloaded.
	   */
	  virtual void userRequestedAbort( unsigned lineNo_r );

	protected:
	  /** Prefix exception message with line information. */
	  std::string errPrefix( unsigned lineNo_r,
				 const std::string & msg_r = std::string(),
				 const std::string & line_r = "-" ) const;

	private:
	  class Impl;
	  RW_pointer<Impl,rw_pointer::Scoped<Impl> > _pimpl;
	  RepoIndexConsumer _repoIndexConsumer;
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
#endif // ZYPP_PARSER_SUSETAGS_CONTENTFILEREADER_H
