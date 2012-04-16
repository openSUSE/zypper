/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/IOStream.h
 *
*/
#ifndef ZYPP_BASE_IOSTREAM_H
#define ZYPP_BASE_IOSTREAM_H

#include <iosfwd>
#include <boost/io/ios_state.hpp>

#include "zypp/base/Flags.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/SafeBool.h"
#include "zypp/base/Function.h"
#include "zypp/base/NonCopyable.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  /** Iostream related utilities.
  */
  namespace iostr
  { /////////////////////////////////////////////////////////////////

    /** Save and restore streams \c width, \c precision
     * and \c fmtflags.
     */
    typedef boost::io::ios_base_all_saver IosFmtFlagsSaver;


    /** Read one line from stream.
     *
     * Reads everything up to the next newline or EOF. newline
     * is read but not returned.
     *
     * \see \ref forEachLine
     */
    std::string getline( std::istream & str );

    /** Copy istream to ostream.
     * \return reference to the ostream.
     */
    inline std::ostream & copy( std::istream & from_r, std::ostream & to_r )
    {
      if ( from_r && to_r )
      {
        char ch;
        while ( from_r && from_r.get( ch ) )
          to_r.put( ch );
      }
      return to_r;
    }

    /** Copy istream to ostream, prefixing each line with \a indent_r (default <tt>"> "</tt> ).
     * \return reference to the ostream.
     */
    inline std::ostream & copyIndent( std::istream & from_r, std::ostream & to_r, const std::string & indent_r = "> " )
    {
      if ( from_r && to_r )
      {
        char ch;
        bool indent = true;
        while ( from_r && from_r.get( ch ) )
        {
          if ( indent )
            to_r << indent_r;
          indent = ( ch == '\n' );
          to_r.put( ch );
        }
      }
      return to_r;
    }

    /** Copy istream to ostream, prefixing each line with \a indent_r (default <tt>"> "</tt> ).
     * \return reference to the ostream.
     */
    inline void tee( std::istream & from_r, std::ostream & to1_r, std::ostream & to2_r )
    {
      if ( from_r && ( to1_r ||to2_r ) )
      {
        char ch;
        while ( from_r && from_r.get( ch ) )
        {
          to1_r.put( ch );
          to2_r.put( ch );
        }
      }
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : EachLine
    //
    /** Simple lineparser: Traverse each line in a file.
     *
     * \code
     * std::ifstream infile( "somefile" );
     * for( iostr::EachLine in( infile ); in; in.next() )
     * {
     *   DBG << *in << endl;
     * }
     * \endcode
     */
    class EachLine : private base::SafeBool<EachLine>, private base::NonCopyable
    {
      typedef base::SafeBool<EachLine> SafeBool;

      public:
	/** Ctor taking a stream and reading the 1st line from it. */
	EachLine( std::istream & str_r, unsigned lineNo_r = 0 );

	/** Evaluate class in a boolean context. */
	using SafeBool::operator bool_type;

	/** Whether \c this contains a valid line to consume. */
	bool valid() const
	{ return boolTest(); }

	/** Return the current line number. */
	unsigned lineNo() const
	{ return _lineNo; }

	std::streamoff lineStart() const
	{ return _lineStart; };

	/** Set current line number. */
	void setLineNo( unsigned lineNo_r )
	{ _lineNo = lineNo_r; }

	/** Access the current line. */
	const std::string & operator*() const
	{ return _line; }
	/** \overload non const access */
	std::string & operator*()
	{ return _line; }

	/** Access the current line. */
	const std::string * operator->() const
	{ return &_line; }

	/** Advance to next line. */
	bool next();

	/** Advance \a num_r lines. */
	bool next( unsigned num_r )
	{
	  while ( num_r-- && next() )
	    ; /* EMPTY */
	  return valid();
	}

      private:
	friend SafeBool::operator bool_type() const;
	bool boolTest() const
	{ return _valid; }

      private:
	std::istream & _str;
	std::string    _line;
	std::streamoff _lineStart;
	unsigned       _lineNo;
	bool           _valid;
    };
    ///////////////////////////////////////////////////////////////////

    /** Simple lineparser: Call functor \a consume_r for each line.
     *
     * \param str_r istream to read from.
     * \param consume_r callback function taking linenumber and content
     *
     * The loop is aborted if the callback  returns \c false.
     *
     * \code
     *   iostr::forEachLine( InputStream( "/my/file/to/read.txt" ),
     *                       []( int num_r, std::string line_r )->bool
     *                       {
     *                         MIL << " [" num_r << "]'" << line_r << "'" << endl;
     *                         return true;
     *                       } );
     * \endcode
     *
     * \return Number if lines consumed (negative if aborted by callback).
     */
     int forEachLine( std::istream & str_r, function<bool(int, std::string)> consume_r );

     /** \ref simpleParseFile modifications before consuming a line. */
     enum ParseFlag
     {
       PF_LTRIM			= 1 << 0,		//< left trim whitespace
       PF_RTRIM			= 1 << 1,		//< right trim whitespace
       PF_TRIM			= PF_LTRIM | PF_RTRIM,	//< trim whitespace
       PF_SKIP_EMPTY		= 1 << 2, 		//< skip lines containing whitespace only
       PF_SKIP_SHARP_COMMENT	= 1 << 3		//< skip lines beginning with '#'
     };
     ZYPP_DECLARE_FLAGS( ParseFlags, ParseFlag );
     ZYPP_DECLARE_OPERATORS_FOR_FLAGS( ParseFlags );

     /** Simple lineparser optionally trimming and skipping comments. */
     int simpleParseFile( std::istream & str_r, ParseFlags flags_r, function<bool(int, std::string)> consume_r );

     /** \overload trimming lines, skipping '#'-comments and empty lines. */
     inline int simpleParseFile( std::istream & str_r, function<bool(int, std::string)> consume_r )
     { return simpleParseFile( str_r, PF_TRIM | PF_SKIP_EMPTY | PF_SKIP_SHARP_COMMENT , consume_r ); }

    /////////////////////////////////////////////////////////////////
  } // namespace iostr
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_IOSTREAM_H
