/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/StrMatcher.h
 *
*/
#ifndef ZYPP_BASE_STRMATCHER_H
#define ZYPP_BASE_STRMATCHER_H

#include <iosfwd>
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/Exception.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  /// \class Match
  /// \brief String matching option flags as used e.g. by \ref StrMatcher.
  ///
  /// \code
  /// Match mode( Match::GLOB | Match::NOCASE );
  /// \endcode
  ///////////////////////////////////////////////////////////////////
  class Match
  {
  private:
    static const int _modemask;
    static const int _flagmask;

  public:
    /** Mode flags (mutual exclusive). */
    enum Mode
    {
      NOTHING,		//!< Match nothing
      STRING,		//!< Excat matching
      STRINGSTART,	//!< Match at string start
      STRINGEND,	//!< Match at string end
      SUBSTRING,	//!< Match substring
      GLOB,		//!< Glob
      REGEX,		//!< Regular Expression
      OTHER		//!< Something else.
    };

    /** \name \ref Option flags
     * Most flags are actually \ref sat::LookupAttr specific, as they tell
     * how to retrieve the attribute values from solv-files. The plain
     * \ref StrMatcher will ignore those flags and use the ones related
     * to string matching only.
     */
    //@{
      static const Match NOCASE;		//!< If set, match case insensitive.
    //@}

    /** \name \ref sat::LookupAttr option flags
     * These flags are actually \ref sat::LookupAttr specific, as they tell
     * how to retrieve the attribute values from solv-files. The plain
     * \ref StrMatcher will ignore these flags and use the ones related
     * to string matching only.
     */
    //@{
      static const Match NO_STORAGE_SOLVABLE;	//!< LookupAttr: internal
      static const Match SUB;			//!< LookupAttr: internal
      static const Match ARRAYSENTINEL;		//!< LookupAttr: internal
      static const Match DISABLED_REPOS;	//!< LookupAttr: internal
      static const Match COMPLETE_FILELIST;	//!< LookupAttr: internal
      static const Match SKIP_KIND;		//!< LookupAttr: skip any \c kind: prefix when looking at a \ref Solvable name.
      static const Match FILES;			//!< LookupAttr: match full path when matching in filelists, otherwise just the basenames.
      static const Match CHECKSUMS;		//!< LookupAttr: also look for matches in checksums
    //@}

  public:
    /** Default ctor \c 0 or \ref NOTHING. */
    Match()
    : _val( 0 )
    {}

    /** Ctor from \ref Mode value. */
    Match( Mode val_r )
    : _val( modeval( val_r ) )
    {}

    /** Just in case one needs it. */
    explicit Match( int val_r )
    : _val( val_r )
    {}

    /** Evaluate in a boolean context <tt>( != 0 )</tt>. */
    explicit operator bool() const
    { return _val; }

  public:
    /** Test whether \c all of the \a rhs bits are set (same mode if \a rhs has one). */
    bool test( const Match & rhs ) const
    { return ( ( flagval() & rhs.flagval() ) == rhs.flagval() ) && ( !rhs.modeval() || rhs.modeval() == modeval() ); }

    /** Whether at least one of the \a rhs bits is set (or the same mode). */
    bool testAnyOf( const Match & rhs ) const
    { return ( flagval() & rhs.flagval() ) || ( rhs.modeval() && rhs.modeval() == modeval() ); }

    /** Set all of the \a rhs bits (setting a new mode if \a rhs has one). */
    void set( const Match & rhs )
    {
      if ( rhs.modeval() )
	_val = rhs._val | flagval(); // also set the rhs mode
	else
	  _val |= rhs._val; // just set the flags
    }

    /** Unset all of the \a rhs bits (unsets mode if the same as \a rhs). */
    void unset( const Match & rhs )
    {
      if ( modeval() == rhs.modeval() )
	_val = flagval() & ~rhs.flagval(); // also unset mode
	else
	  _val &= ~rhs.flagval(); // just unset falgs
    }

    /** Depending on the value of \a onoff, set or unset flags. */
    void turn( const Match & rhs, bool onoff )
    { onoff ? set( rhs ) : unset( rhs ); }

    /** Add flags. */
    Match & operator|=( const Match & rhs )
    { set( rhs ); return *this; }

    /** Remove flags.*/
    Match & operator-=( const Match & rhs )
    { unset( rhs ); return *this; }

  public:
    /** Return the \c mode part. */
    Mode mode() const;

    /** Return the \c flags part. */
    Match flags() const
    { return Match( flagval() ); }

  public:
    /** \name Low level integer representation. */
    //@{
      /** Return the integer representation. */
      int get() const		{ return _val; }
      /** Return the modes integer representation. */
      int modeval() const	{ return _val & _modemask; }
      /** Return the flags integer representation. */
      int flagval() const	{ return _val & _flagmask; }
      //@}

  public:
    /** \name Mode flag manip/query convenience. */
    //@{
      /** Whether this has mode \a rhs */
      bool isMode( Mode rhs ) const
      { return modeval() == modeval( rhs ); }
      /** Whether this has mode \ref STRING. */
      bool isModeString() const
      { return isMode( STRING ); }
      /** Whether this has mode \ref STRINGSTART. */
      bool isModeStringstart() const
      { return isMode( STRINGSTART ); }
      /** Whether this has mode \ref STRINGEND. */
      bool isModeStringend() const
      { return isMode( STRINGEND ); }
      /** Whether this has mode \ref SUBSTRING. */
      bool isModeSubstring() const
      { return isMode( SUBSTRING ); }
      /** Whether this has mode \ref GLOB. */
      bool isModeGlob() const
      { return isMode( GLOB ); }
      /** Whether this has mode \ref REGEX. */
      bool isModeRegex() const
      { return isMode( REGEX ); }

      /** Set the mode part to \a rhs . */
      void setMode( Mode rhs )
      { _val = modeval( rhs ) | flagval(); }
      /** Set the mode \ref STRING. */
      void setModeString()
      { setMode( STRING ); }
      /** Set the mode \ref STRINGSTART. */
      void setModeStringstart()
      { setMode( STRINGSTART ); }
      /** Set the mode \ref STRINGEND. */
      void setModeStringend()
      { setMode( STRINGEND ); }
      /** Set the mode \ref SUBSTRING. */
      void setModeSubstring()
      { setMode( SUBSTRING ); }
      /** Set the mode \ref GLOB. */
      void setModeGlob()
      { setMode( GLOB ); }
      /** Set the mode \ref REGEX. */
      void setModeRegex()
      { setMode( REGEX ); }
      //@}

      /** String representation. */
      std::string asString() const;

  private:
    /** Numeric value for enum (short for <tt>Match(m).get()</tt>). */
    static int modeval( Mode mode_r );

  private:
    int _val;
  };

  /** \relates Match */
  inline bool operator==( const Match & lhs, const Match & rhs )
  { return lhs.get() == rhs.get(); }
  /** \relates Match */
  inline bool operator!=( const Match & lhs, const Match & rhs )
  { return lhs.get() != rhs.get(); }

  /** \relates Match */
  inline Match operator|( const Match & lhs, const Match & rhs )
  { return Match(lhs) |= rhs; }
  /** \relates Match \overload to disambiguate 'int|int'. */
  inline Match operator|( Match::Mode lhs, Match::Mode rhs )
  { return Match(lhs) |= rhs; }

  /** \relates Match */
  inline Match operator-( const Match & lhs, const Match & rhs )
  { return Match(lhs) -= rhs; }
  /** \relates Match \overload to disambiguate 'int-int'. */
  inline Match operator-( Match::Mode lhs, Match::Mode rhs )
  { return Match(lhs) -= rhs; }

  /** \relates Match::Mode Stream output */
  std::ostream & operator<<( std::ostream & str, Match::Mode obj );

  /** \relates Match Stream output */
  std::ostream & operator<<( std::ostream & str, const Match & obj );

  ///////////////////////////////////////////////////////////////////
  /// \class MatchException
  /// \brief Exceptions thrown from attribute matching.
  ///////////////////////////////////////////////////////////////////
  struct MatchException : public Exception
  {
    /** Supplied message. */
    explicit MatchException( const std::string & msg_r ) : Exception( msg_r ) {}
  };

  ///////////////////////////////////////////////////////////////////
  /// \class MatchUnknownModeException
  /// \brief Unknown match mode.
  ///////////////////////////////////////////////////////////////////
  struct MatchUnknownModeException : public MatchException
  {
    /** Supplied message. */
    explicit MatchUnknownModeException( const std::string & msg_r ) : MatchException( msg_r ) {}

    /** Build message including the \a mode and optional the pattern string. */
    MatchUnknownModeException( const Match & mode_r, const std::string & msg_r = std::string() );
  };

  ///////////////////////////////////////////////////////////////////
  /// \class MatchInvalidRegexException
  /// \brief Invalid regular expression (failed ::regcomp).
  ///////////////////////////////////////////////////////////////////
  struct MatchInvalidRegexException : public MatchException
  {
    /** Supplied message. */
    explicit MatchInvalidRegexException( const std::string & msg_r ) : MatchException( msg_r ) {}

    /** Build message including the \a regex and \c ::regcomp returncode (use \c 0 if unknown). */
    MatchInvalidRegexException( const std::string & regex_r, int regcomp_r );
  };

  ///////////////////////////////////////////////////////////////////
  /// \class StrMatcher
  /// \brief String matching (STRING|SUBSTRING|GLOB|REGEX).
  ///
  /// Used by e.g. \ref PoolQuery and \ref LookupAttr for queries,
  /// but it can also be used for matching arbitrary strings.
  ///
  /// \code
  ///  StrMatcher matches( "foo", Match::SUBSTRING );
  ///  for_( it, stringlist.begin(), stringlist().end() )
  ///  {
  ///    if ( matches( *it ) )
  ///      cout << *it << " has substring 'foo'" << endl;
  ///  }
  /// \endcode
  ///
  /// \Note Those flags are always set: <tt>REG_EXTENDED | REG_NOSUB | REG_NEWLINE</tt>
  ///////////////////////////////////////////////////////////////////
  class StrMatcher
  {
    friend std::ostream & operator<<( std::ostream & str, const StrMatcher & obj );

  public:
    typedef MatchException Exception;

  public:
    /** Implementation  */
    class Impl;

  public:
    /** Default ctor matches nothing. */
    StrMatcher();

    /** Ctor from string matches in \ref Match::STRING mode per default. */
    StrMatcher( const std::string & search_r );

    /** Ctor taking string and \ref Match flags. */
    StrMatcher( const std::string & search_r, const Match & flags_r );

    /** Ctor taking string and \ref Match::Mode.
     * Needed because we want them to be treated as \ref Match,
     * and not as \ref int as the compiler woud do.
     */
    StrMatcher( const std::string & search_r, const Match::Mode & flags_r );

    /** Low level interface wraps \a flags into \ref Match. */
    StrMatcher( const std::string & search_r, int flags_r );

    /** Evaluate in a boolean context <tt>( ! searchstring().empty() )</tt>. */
    explicit operator bool() const
    { return !searchstring().empty(); }

  public:
    /** Return whether string matches.
     * You can use it with any class that impements \c c_str.
     * (\c std::string, \ref Pathname, \ref IdString, ...).
     * \Note \c NULL never matches.
     */
    template<class Tp>
    bool operator()( const Tp & string_r ) const
    { return doMatch( string_r.c_str() ); }
    /** \overload */
    bool operator()( const char * string_r ) const
    { return doMatch( string_r ); }

  public:
    /** The current searchstring. */
    const std::string & searchstring() const;

    /** Set a new searchstring. */
    void setSearchstring( const std::string & string_r );

    /** Set a new searchstring and flags. */
    void setSearchstring( const std::string & string_r, const Match & flags_r );

    /** The current search flags. */
    const Match & flags() const;

    /** Set new search flags. */
    void setFlags( const Match & flags_r );

  public:
    /** Compile the pattern e.g. in case of \c REGEX.
     * \throws MatchUnknownModeException If the \ref Match flag more than
     *         one mode bit set.
     * \throws MatchInvalidRegexException If \ref Match::REGEX is set
     *         and \ref searchstring is not a valid regular expression.
     */
    void compile() const;

    /** Whether the \ref StrMatcher is already compiled. */
    bool isCompiled() const;

    /** Return whether string matches.
     * Compiles the \ref StrMatcher if this was not yet done.
     * \throws MatchException Any of the exceptions thrown by \ref StrMatcher::compile.
     */
    bool doMatch( const char * string_r ) const;

  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };

  /** \relates StrMatcher Stream output */
  std::ostream & operator<<( std::ostream & str, const StrMatcher & obj );

  /** \relates StrMatcher */
  bool operator==( const StrMatcher & lhs, const StrMatcher & rhs );

  /** \relates StrMatcher */
  inline bool operator!=( const StrMatcher & lhs, const StrMatcher & rhs )
  { return !( lhs == rhs ); }

  /** \relates StrMatcher Arbitrary order for std::container. */
  bool operator<( const StrMatcher & lhs, const StrMatcher & rhs );

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_STRMATCHER_H
