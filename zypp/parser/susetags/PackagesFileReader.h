/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/susetags/PackagesFileReader.h
 *
*/
#ifndef ZYPP_PARSER_SUSETAGS_PACKAGESFILEREADER_H
#define ZYPP_PARSER_SUSETAGS_PACKAGESFILEREADER_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/parser/TagParser.h"

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
      //	CLASS NAME : PackagesFileReader
      //
      /** */
      class PackagesFileReader : public TagParser
      {
	public:
	  /** Default ctor */
	  PackagesFileReader();
	  /** Dtor */
	  virtual ~PackagesFileReader();

	private:
	  /** Called when start parsing. */
	  virtual void beginParse();
	  /** Called when a single-tag is found. */
	  virtual void consume( const SingleTagPtr & tag_r );
	  /** Called when a multi-tag is found. */
	  virtual void consume( const MultiTagPtr & tag_r );
	  /** Called when the parse is done. */
	  virtual void endParse();

	private:
	  class Impl;
	  scoped_ptr<Impl> _pimpl;
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
#endif // ZYPP_PARSER_SUSETAGS_PACKAGESFILEREADER_H
