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

#include "zypp/base/PtrTypes.h"
#include <zypp/base/SafeBool.h>
#include <zypp/base/NonCopyable.h>

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
     * \param str_r The istream to read from.
     * \param consume_r A reference to a function or functor. The loop is
     * aborted if the function returns \c false.
     * \code
     * bool consume( const std::string & )
     * { ... }
     *
     * struct Consume : public std::unary_function<const std::string &, bool>
     * {
     *   bool operator()( const std::string & line_r )
     *   { ... }
     * };
     * \endcode
     *
     * \return A reference to \a consume_r.
     *
     * \todo Should be templated and specialized according to the
     * functors return type, to allow \c void consumer.
     */
    template<class _Function>
      _Function & forEachLine( std::istream & str_r, _Function & consume_r )
      {
        while ( str_r )
          {
            std::string l = getline( str_r );
            if ( ! (str_r.fail() || str_r.bad()) )
              {
                // l contains valid data to be consumed.
                if ( ! consume_r( l ) )
                  break;
              }
          }
        return consume_r;
      }

    /////////////////////////////////////////////////////////////////
  } // namespace iostr
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_IOSTREAM_H
