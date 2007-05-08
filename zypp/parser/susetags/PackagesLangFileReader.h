/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/susetags/PackagesLangFileReader.h
 *
*/
#ifndef ZYPP_PARSER_SUSETAGS_PACKAGESLANGFILEREADER_H
#define ZYPP_PARSER_SUSETAGS_PACKAGESLANGFILEREADER_H

#include <iosfwd>

#include "zypp/parser/susetags/FileReaderBase.h"
#include "zypp/Locale.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace data
  { /////////////////////////////////////////////////////////////////
    class Package;
    DEFINE_PTR_TYPE(Package);
    class SrcPackage;
    DEFINE_PTR_TYPE(SrcPackage);
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
      //	CLASS NAME : PackagesLangFileReader
      //
      /** */
      class PackagesLangFileReader : public FileReaderBase
      {
	public:
	  typedef function<void(const data::Package_Ptr &)> PkgConsumer;
	  typedef function<void(const data::SrcPackage_Ptr &)> SrcPkgConsumer;

	public:
	  /** Default ctor */
	  PackagesLangFileReader();
	  /** Dtor */
	  virtual ~PackagesLangFileReader();

	public:
	  /** Locale to parse. */
	  void setLocale( const Locale & locale_r )
	  { _locale = locale_r; }

	  /** Consumer to call when a (binary) package entry was parsed. */
	  void setPkgConsumer( const PkgConsumer & fnc_r )
	  { _pkgConsumer = fnc_r; }

	  /** Consumer to call when a (source) package entry was parsed. */
	  void setSrcPkgConsumer( const SrcPkgConsumer & fnc_r )
	  { _srcPkgConsumer = fnc_r; }

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
	  Locale           _locale;
	  PkgConsumer      _pkgConsumer;
	  SrcPkgConsumer   _srcPkgConsumer;
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
#endif // ZYPP_PARSER_SUSETAGS_PACKAGESLANGFILEREADER_H
