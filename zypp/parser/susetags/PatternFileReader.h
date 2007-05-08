/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/susetags/PatternFileReader.h
 *
*/
#ifndef ZYPP_PARSER_SUSETAGS_PATTERNFILEREADER_H
#define ZYPP_PARSER_SUSETAGS_PATTERNFILEREADER_H

#include <iosfwd>

#include "zypp/parser/susetags/FileReaderBase.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace data
  { /////////////////////////////////////////////////////////////////
    class Pattern;
    DEFINE_PTR_TYPE(Pattern);
    /////////////////////////////////////////////////////////////////
  } // namespace data
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : PatternFileReader
      //
      /** */
      class PatternFileReader : public FileReaderBase
      {
	public:
	  typedef function<void(const data::Pattern_Ptr &)> Consumer;

	public:
	  /** Default ctor */
	  PatternFileReader();
	  /** Dtor */
	  virtual ~PatternFileReader();

	public:
	  /** Consumer to call when a pattern was parsed. */
	  void setConsumer( const Consumer & fnc_r )
	  { _consumer = fnc_r; }

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
	  Consumer         _consumer;
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
#endif // ZYPP_PARSER_SUSETAGS_PATTERNFILEREADER_H
