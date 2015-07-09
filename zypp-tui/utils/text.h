/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_UTILS_TEXT_H_
#define ZYPPER_UTILS_TEXT_H_

#include <iosfwd>
#include <string>

#include <boost/utility/string_ref.hpp>
#include <zypp/base/DtorReset.h>

#include "output/Out.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace mbs
{
#define ZYPPER_TRACE_MBS 0
  ///////////////////////////////////////////////////////////////////
  /// \class MbsIterator
  /// \brief Iterate over chars and ANSI SGR in a multi-byte character string
  ///////////////////////////////////////////////////////////////////
  struct MbsIterator
  {
    MbsIterator( boost::string_ref text_r )
    : _text( text_r )
    , _tpos( _text.data() )
    , _trest( _text.size() )
    , _tread( 0 )
    , _wc( L'\0' )
    { memset( &_mbstate, 0, sizeof(_mbstate) ); operator++(); }

    /** Use with care; all WS are  faked to either '\n' or ' ' */
    wchar_t & operator*()		{ return _wc; }
    const wchar_t & operator*() const	{ return _wc; }

    const char * pos() const		{ return _tpos; }
    size_t       size() const		{ return _tread; }
    size_t       columns() const	{ size_t ret = ::wcwidth(_wc); if ( ret == size_t(-1) ) ret = 0; return ret; }

    boost::string_ref ref() const	{ return boost::string_ref( _tpos, _tread ); }

    bool atEnd() const			{ return _trest == 0; }
    bool isNL() const			{ return( _wc == L'\n' ); }
    bool isWS() const			{ return( _wc == L' ' ); }
    bool isCH() const			{ return !( atEnd() || isNL() || isWS() ); }

    MbsIterator & operator++()
    {
      if ( !atEnd() )
      {
	_tpos += _tread;
	_trest -= _tread;
	_tread = ::mbrtowc( &_wc, _tpos, _trest, &_mbstate );

	switch ( _tread )
	{
	  case (size_t)-2:
	  case (size_t)-1:
	    _tread = 0;
	    // fall through
	  case 0:
	    _trest = 0;	// atEnd
	    _wc = L'\0';
	    break;

	  default:
	    if ( ::iswspace(_wc) )
	    {
	      switch ( _wc )
	      {
		case L'\n':
		case L' ':
		  break;
		default:
		  _wc = L' ';
	      }
	    }
	    else if ( _wc == L'\033' )	// ansi SGR ?
	    {
	      unsigned asize = ansiSize( _tpos );
	      if ( asize && asize <= _trest )
		_tread = asize;
	    }
	    break;
	}
      }
      return *this;
    }

  private:
    unsigned ansiSize( const char * pos_r )
    {
      unsigned ret = 0;
      const char * p = pos_r;
      if ( *p == '\033' && *(++p) == '[' )
      {
	for ( char ch = *(++p); ( '0' <= ch && ch <= '9' ) || ch ==';'; ch = *(++p) )
	{;}
	if ( *p == 'm' )
	  ret = p+1 - pos_r;
      }
      return ret;
    }

    boost::string_ref	_text;
    const char *	_tpos;	// start of last ::mbrtowc
    size_t		_trest;	// _tpos to end of string
    size_t		_tread;	// consumed in last ::mbrtowc

    wchar_t		_wc;	// result of last ::mbrtowc
    mbstate_t		_mbstate;
  };

  ///////////////////////////////////////////////////////////////////
  /// \class MbsWriteWrapped
  /// \brief Write MBString optionally wrapped and indented.
  ///
  /// The class assumes the output starts at the beginning of a
  /// new term line. Every '\n' in text starts a new paragraph
  /// (on an new line with default indent).
  ///
  /// WS at par begin increments indent for this par.
  ///
  /// Use \ref gotoNextLine to open a new indented line without
  /// stating a new par.
  ///
  /// If MbsWriteWrapped is used in conjunction with plain screen
  /// output, you can use \ref resetToParBegin to reset housekeeping
  /// data after plain screen output.
  ///////////////////////////////////////////////////////////////////
  struct MbsWriteWrapped
  {
    MbsWriteWrapped( std::ostream & out )
    : MbsWriteWrapped( out, 0, 0 )
    {}

    MbsWriteWrapped( std::ostream & out, size_t wrap_r )
    : MbsWriteWrapped( out, 0, wrap_r )
    {}

    MbsWriteWrapped( std::ostream & out, size_t indent_r, size_t wrap_r, int indentFix_r = 0 )
    : _out( out )
    , _defaultWrap( wrap_r )
    , _defaultIndent( indent_r )
    , _defaultIndentFix( indentFix_r )
    , _indent( saneIncrementIndent( 0, _defaultIndent, _defaultWrap ) )
    , _indentFix( _defaultIndentFix )
    , _indentGap( 0 )
    , _lpos( 0 )
    , _gap( 0 )
    , _gapForced( 0 )
    , _gapLines( 0 )
    , _word( nullptr )
    , _wSize( 0 )
    , _wColumns( 0 )
    {}

    size_t defaultWrap() const		{ return _defaultWrap; }
    size_t defaultIndent() const	{ return _defaultIndent; }
    int    defaultIndentFix() const	{ return _defaultIndentFix; }

    size_t indent() const		{ return _indent; }
    size_t lpos() const			{ return _lpos; }

    bool atLineBegin() const		{ return( _lpos == 0 ); }
    bool atParBegin() const		{ return( _lpos == 0 && !_gapLines ); }

    /** Reset housekeeping data to the beginning of a paragraph (does not write anything) */
    void resetToParBegin()
    {
      clearGap();
      clearWord();
      clearIndent();
      _lpos = 0;
    }


    /** Write out any pending word and start a new par (NL unconditionally) */
    void gotoNextPar()
    {
      writeout(true);
      clearIndent();
#if ( ZYPPER_TRACE_MBS)
      _out << "<NL>" << endl;	// "<NL>"
#else
      _out << endl;	// "<NL>"
#endif
      _lpos = 0;
    }

    /** Open a new paragraph if not \ref atParBegin */
    void gotoParBegin()
    { if ( ! atParBegin() ) gotoNextPar(); }


    /** Add \a count_r (1) new lines in this par (BR unconditionally) */
    void gotoNextLine( size_t count_r = 1 )
    {
      if ( count_r )
      {
	writeout(true);	// but keep indent
	_gapLines += count_r;
#if ( ZYPPER_TRACE_MBS )
	while ( count_r-- )
	  _out << "<BR>" << endl;	// "<BR>"
#else
	_out << std::string( count_r, '\n' );
#endif
	_lpos = 0;
      }
    }

    /** Open a new line in this paragraph if not \ref atLineBegin */
    void gotoLineBegin()
    { if ( ! atLineBegin() ) gotoNextLine(); }


    /** Temporarily increase indent */
    struct ScopedIndentIncrement : private zypp::DtorReset
    {
      ScopedIndentIncrement( MbsWriteWrapped & mww_r, size_t increment_r )
      : zypp::DtorReset( mww_r._indent )
      { mww_r._indent = mww_r.saneIncrementIndent( increment_r ); }
    };
    /** \relates ScopedIndentIncrement Temporarily increase indent. */
    ScopedIndentIncrement scopedIndentIncrement( size_t increment_r )
    { return ScopedIndentIncrement( *this, increment_r ); }


    /** Write \a text_r; starting a new paragraph. */
    void startPar( boost::string_ref text_r )
    {
      gotoParBegin();
      write( text_r );
    }
    /** \overload Indented */
    void startPar( boost::string_ref text_r, size_t increment_r )
    { ScopedIndentIncrement s1( *this, increment_r ); startPar( text_r ); }


    /** Continue writing text at the current position. */
    void addString( boost::string_ref text_r )
    {
      write( text_r );
    }
    /** \overload Indented */
    void addString( boost::string_ref text_r, size_t increment_r )
    { ScopedIndentIncrement s1( *this, increment_r ); addString( text_r ); }


    /** Continue writing text (separated by WS if not \ref atLineBegin). */
    void writeText( boost::string_ref text_r )
    {
      _gapForced = 1;
      write( text_r );
    }
    /** \overload Indented */
    void writeText( boost::string_ref text_r, size_t increment_r )
    { ScopedIndentIncrement s1( *this, increment_r ); writeText( text_r ); }


    /** Write \a text_r; starting a new paragraph and ending it after the text was written. */
    void writePar( boost::string_ref text_r )
    {
      gotoParBegin();
      write( text_r );
      gotoParBegin();
    }
    /** \overload Indented */
    void writePar( boost::string_ref text_r, size_t increment_r )
    { ScopedIndentIncrement s1( *this, increment_r ); writePar( text_r ); }


    /** Write a \a tag_r with indented definition \a text_r.
     * Optional \a tagincr_r (0) and \a textincr_r (28) are counted on
     * top of the base increment.
     */
    void writeDefinition( boost::string_ref tag_r, boost::string_ref text_r, size_t tagincr_r, size_t textincr_r )
    {
      gotoParBegin();
      {
	ScopedIndentIncrement s1( *this, tagincr_r );
	write( tag_r, /*leadingWSindents_r*/false );
      }
      {
	ScopedIndentIncrement s1( *this, textincr_r );
	if ( _lpos < _indent )
	  _gapForced = _indent - _lpos;
	else
	  gotoNextLine();
	write( text_r );
      }
      gotoParBegin();
    }
    /** \overload Use default tag/text increment */
    void writeDefinition( boost::string_ref tag_r, boost::string_ref text_r )
    { writeDefinition( tag_r, text_r, 0, 28 ); }
    /** \overload Indented */
    void writeDefinition( boost::string_ref tag_r, boost::string_ref text_r, size_t tagincr_r, size_t textincr_r, size_t increment_r )
    { ScopedIndentIncrement s1( *this, increment_r ); writeDefinition( tag_r, text_r, tagincr_r, textincr_r ); }
    /** \overload Indented using default tag/text increment */
    void writeDefinition( boost::string_ref tag_r, boost::string_ref text_r, size_t increment_r )
    { ScopedIndentIncrement s1( *this, increment_r ); writeDefinition( tag_r, text_r ); }


  private:
    /** Append \a text_r indented and wrapped at the current position.
     * All words and NL are written, only trailing WS is remembered.
     */
    void write( boost::string_ref text_r, bool leadingWSindents_r = true )
    {
      for( MbsIterator it( text_r ); ! it.atEnd(); ++it )
      {
	if ( it.isNL() )
	{
	  gotoNextPar();	// write out any pending word and start new par
	}
	else if ( it.isWS() )
	{
	  if ( _word )		// write out pending word and start new gap
	  {
	    writeout();
	    ++_gap;
	  }
	  else
	  {
	    if ( atParBegin() && leadingWSindents_r )	// ws at par begin may increment indent
	      ++_indentGap;
	    else
	      ++_gap;
	  }
	}
	else // non WS		// remember in word
	{
	  if ( !_word )
	    _word = it.pos();
	  _wSize += it.size();
	  _wColumns += it.columns();
	}
      }
      writeout();		// write out any pending word; gaps are remembered for next text
    }

    /** Write any pending "indent/gap+word" and reset for next word.
     * If \a force_r, gap is cleared even if no word is pending. This
     * is used before writing a '\n'.
     */
    void writeout( bool force_r = false )
    {
      if ( _word )
      {
	_writeoutPending();
	// reset gaps and word
	clearGap();
	clearWord();
      }
      else if ( force_r )
	clearGap();
    }

    void _writeoutPending()
    {
      // NOTE: we are either atLineBegin or in a _gap between words
      if ( !atLineBegin() )
      {
	if ( _gap < _gapForced )
	  _gap = _gapForced;
	if ( !_defaultWrap || _lpos+_gap+_wColumns <= _defaultWrap )
	{
	  _out << std::string( _gap, ' ' ) <<  boost::string_ref( _word, _wSize );
	  _lpos += _gap + _wColumns;
	  return;
	}
	// Here: did not fit on this line
	// suppress gap and write indented on next line
	clearGap();
	_out << endl;
	_lpos = 0;
      }

      // Here: atLineBegin
      unsigned useIndent = fixIndent( _indent + _indentGap + _gap );

      if ( _defaultWrap ) // fix large indent
      { while ( useIndent >= _defaultWrap ) useIndent -= _defaultWrap; }

      // Try writing the whole word
      if ( !_defaultWrap || useIndent+_wColumns <= _defaultWrap )
      {
	_out << std::string( useIndent, ' ' ) <<  boost::string_ref( _word, _wSize );
	_lpos += useIndent + _wColumns;
	return;
      }

      // Still here: word is too big, we need to split it :(
      for( MbsIterator it( boost::string_ref( _word, _wSize ) ); ! it.atEnd(); ++it )
      {
	if ( atLineBegin() )
	{
	  _out << std::string( useIndent, ' ' );
	  _lpos += useIndent;
	}
	_out << it.ref();
	++_lpos;
	if ( _lpos >= _defaultWrap )
	{
	  _out << endl;
	  _lpos = 0;
	}
      }
    }

    /** Return fixed indent_r */
    static size_t fixIndent( size_t indent_r, int indentFix_r )
    {
      if ( indentFix_r )
      {
	if ( indentFix_r < 0 && ( size_t(-indentFix_r) >= indent_r ) )
	  indent_r = 0;
	else
	  indent_r += indentFix_r;
      }
     return indent_r;
    }

    /** Return fixed indent_r (unsets _indentFix) */
    size_t fixIndent( size_t indent_r )
    {
      indent_r = fixIndent( indent_r, _indentFix );
      _indentFix = 0;
      return indent_r;
    }

    /** Return incremented indent, but not to more than 50% of the remaining line size if wrapped.  */
    static size_t saneIncrementIndent( size_t current_r, size_t increment_r, size_t wrap_r )
    {
      if ( ! increment_r )
	return current_r;

      increment_r += current_r;
      if ( wrap_r && current_r < wrap_r )
      {
	size_t limit = current_r + ( (wrap_r-current_r)/2 );
	if ( limit < increment_r )
	  increment_r = limit;
      }
      return increment_r;
    }

    /** Return incremented \c _indent, but not more than 50% of the remaining line size if wrapped.  */
    size_t saneIncrementIndent( size_t increment_r )
    { return saneIncrementIndent( _indent, increment_r, _defaultWrap ); }

    /** Set default indent at par start (reloads \c _indentFix) */
    void clearIndent()
    { _indentFix = _defaultIndentFix; _indentGap = 0; }

    /** Set no gaps */
    void clearGap()
    { _gap = 0; _gapForced = 0; _gapLines = 0; }

    /** Set no word pending */
    void clearWord()
    { _word = nullptr; _wSize = 0; _wColumns = 0; }

    std::ostream &	_out;
    const size_t	_defaultWrap;
    const size_t	_defaultIndent;
    const int		_defaultIndentFix;

    size_t _indent;	// base indent for par
    int    _indentFix;	// indent correction for a pars 1st line
    size_t _indentGap;	// additional indent for current par

    size_t _lpos;	// cursor pos on current line

    size_t _gap;	// amount of WS before next word
    size_t _gapForced;	// forced WS before next word
    size_t _gapLines;	// forced NL before next word/par

    const char * _word;	// current word start in text
    size_t _wSize;	// current word size in byte
    size_t _wColumns;	// current word screen columns
  };
#undef ZYPPER_TRACE_MBS
} // namespace mbs
///////////////////////////////////////////////////////////////////

/**
 * Wrap and indent given \a text and write it to the output stream \a out.
 *
 * TODO
 * - delete whitespace at the end of lines
 * - keep one-letter words with the next
 *
 * \param out       output stream to write to
 * \param test      text to wrap
 * \param indent    number of columns by which to indent the whole text
 * \param wrap      number of columns the text should be wrapped into
 * \param indentFix additional indent/outdent for the first line (default: \c 0)
 */
inline void mbs_write_wrapped( std::ostream & out, const std::string & text_r, size_t indent_r, size_t wrap_r, int indentFix_r = 0 )
{
  mbs::MbsWriteWrapped mww( out, indent_r, wrap_r, indentFix_r );
  mww.addString( text_r );
}

/** Returns the column width of a multi-byte character string \a str */
unsigned mbs_width (const std::string & str);

/**
 * Returns a substring of a multi-byte character string \a str starting
 * at screen column \a pos and being \a n columns wide, as far as possible
 * according to the multi-column characters found in \a str.
 */
std::string mbs_substr_by_width(
    const std::string & str,
    std::string::size_type pos,
    std::string::size_type n = std::string::npos);

#endif /* ZYPPER_UTILS_TEXT_H_ */
